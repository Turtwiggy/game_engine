#include "pch.hpp"

#include "engine/physics/physics_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/collisions/resolve_collisions_helpers.hpp"
#include "modules/core/raws/raws_components.hpp"

#include <TaskScheduler.h>

namespace game2d {

struct SampleContext
{
  int workerCount = 1;
};

class Sample
{
public:
  static constexpr int m_maxTasks = 64;
  // static constexpr int m_maxThreads = 64;

  SampleContext* m_context;

  enki::TaskScheduler* m_scheduler;
  class SampleTask* m_tasks;
  int m_taskCount;
  int m_threadCount;
};

class SampleTask : public enki::ITaskSet
{
public:
  SampleTask() = default;

  void ExecuteRange(enki::TaskSetPartition range, uint32_t threadIndex) override
  {
    m_task(range.start, range.end, threadIndex, m_taskContext);
  }

  b2TaskCallback* m_task = nullptr;
  void* m_taskContext = nullptr;
};

static void*
EnqueueTask(b2TaskCallback* task, int32_t itemCount, int32_t minRange, void* taskContext, void* userContext)
{
  Sample* sample = static_cast<Sample*>(userContext);
  if (sample->m_taskCount < Sample::m_maxTasks) {
    SampleTask& sampleTask = sample->m_tasks[sample->m_taskCount];
    sampleTask.m_SetSize = itemCount;
    sampleTask.m_MinRange = minRange;
    sampleTask.m_task = task;
    sampleTask.m_taskContext = taskContext;
    sample->m_scheduler->AddTaskSetToPipe(&sampleTask);
    ++sample->m_taskCount;
    return &sampleTask;
  } else {
    // This is not fatal but the maxTasks should be increased
    assert(false);
    task(0, itemCount, 0, taskContext);
    return nullptr;
  }
}

static void
FinishTask(void* taskPtr, void* userContext)
{
  if (taskPtr != nullptr) {
    SampleTask* sampleTask = static_cast<SampleTask*>(taskPtr);
    Sample* sample = static_cast<Sample*>(userContext);
    sample->m_scheduler->WaitforTask(sampleTask);
  }
}

// store one physics world...
static b2WorldId worldId = b2_nullWorldId;
static SampleContext m_context;
static Sample m_sample;
static b2WorldDef world_def;
static bool init = false;

void
physics_reset_task_count()
{
  if (!init)
    return;
  m_sample.m_taskCount = 0;
}

void
emplace_or_replace_physics_world(entt::registry& r)
{
  if (!init) {
    init = true;

    const int maxThreadCount = enki::GetNumHardwareThreads();
    const int half_threads = (int)(maxThreadCount * 0.5f);
    m_context.workerCount = b2ClampInt(half_threads, 1, maxThreadCount);
    SDL_Log("(box2d) workerCount: %i", m_context.workerCount);

    m_sample.m_context = &m_context;
    m_sample.m_scheduler = new enki::TaskScheduler;
    m_sample.m_scheduler->Initialize(m_context.workerCount);
    m_sample.m_tasks = new SampleTask[m_sample.m_maxTasks];
    m_sample.m_taskCount = 0;
    m_sample.m_threadCount = 1 + m_context.workerCount;

    world_def = b2DefaultWorldDef();
    world_def.gravity = { 0.0f, 0.0f };
    world_def.workerCount = m_context.workerCount;
    world_def.enqueueTask = EnqueueTask;
    world_def.finishTask = FinishTask;
    world_def.userTaskContext = &m_sample;
    world_def.enableSleep = true;
    worldId = b2CreateWorld(&world_def);
  }

  // cleanup physics world...
  static bool needs_deleting = false;
  if (needs_deleting) {
    SDL_Log("%s", "cleaning up physics world..");
    b2DestroyWorld(worldId);
    worldId = b2_nullWorldId;
    worldId = b2CreateWorld(&world_def);
  }

  needs_deleting = true;
  SDL_Log("%s", std::format("physics world set to clean up... (needs_deleting: {})", needs_deleting).c_str());

  destroy_first_and_create<SINGLE_Physics>(r, SINGLE_Physics{ worldId });
  r.emplace<Persistent>(get_first<SINGLE_Physics>(r));
};

entt::entity
get_fixture(entt::registry& r, entt::entity e)
{
  const auto& body_c = r.get<PhysicsBodyComponent>(e);
  for (const entt::entity fix_e : body_c.fixtures)
    return fix_e;

  SDL_Log("missing get_fixture() fixture");
  throw std::runtime_error("missing get_fixture_by_tag()");
  return entt::null;
};

entt::entity
get_fixture_by_tag(entt::registry& r, entt::entity e, std::string tag)
{
  const auto& body_c = r.get<const PhysicsBodyComponent>(e);
  for (const entt::entity fix_e : body_c.fixtures) {
    const auto& fix_c = r.get<const PhysicsFixtureComponent>(fix_e);
    // const auto* b2_fixture = fix_c.fixture;
    const auto& fix_tag = r.get<const TagComponent>(fix_e);
    if (fix_tag.tag == tag)
      return fix_e;
  }

  SDL_Log("missing get_fixture_by_tag(): %s", tag.c_str());
  throw std::runtime_error("missing get_fixture_by_tag()");
  exit(1); // explode

  return entt::null;
};

PhysicsFixtureDef
get_fixture_def_by_tag(entt::registry& r, entt::entity e, std::string tag)
{
  const auto& item_key_c = r.get<ItemKey>(e);
  const auto item_c = find_item(r, item_key_c.key);

  std::optional<PhysicsFixtureDef> fixture_def_opt = std::nullopt;
  for (const auto& fix : item_c.phys_fixtures.value())
    if (fix.tag == tag)
      fixture_def_opt = fix;

  if (!fixture_def_opt) {
    throw std::runtime_error("Failed to find fixture def");
    exit(1); // crash
  }

  return fixture_def_opt.value();
};

std::unordered_set<entt::entity>
get_all_in_area(entt::registry& r, b2Vec2 center_m, float d_in_meters)
{
  const auto& physics_c = get_first_component<SINGLE_Physics>(r);

  b2AABB aabb;
  aabb.lowerBound = b2Vec2{ center_m.x - d_in_meters, center_m.y - d_in_meters };
  aabb.upperBound = b2Vec2{ center_m.x + d_in_meters, center_m.y + d_in_meters };

  struct QueryContext
  {
    std::unordered_set<entt::entity> results;
  };
  QueryContext context;

  const auto overlap_callback = [](const b2ShapeId shapeId, void* context) -> bool {
    QueryContext* queryContext = static_cast<QueryContext*>(context);
    const b2BodyId bodyId = b2Shape_GetBody(shapeId);

    const auto eid = entt::entity{ static_cast<entt::id_type>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyId))) };
    queryContext->results.emplace(eid);
    return true; // return true to continue the query
  };

  b2World_OverlapAABB(physics_c.worldId, aabb, b2DefaultQueryFilter(), overlap_callback, &context);
  return context.results;
};

std::unordered_map<entt::entity, std::vector<CollisionWithFixtureResult>>
get_all_in_area_filtered(entt::registry& r,
                         const b2Vec2 center_in_meters,
                         const float d_in_meters,
                         const std::function<bool(entt::registry&, entt::entity)>& cond)
{
  // FilteredSearchAreaCallback callback(r, { center_in_meters.x, center_in_meters.y }, cond);

  b2AABB aabb;
  aabb.lowerBound = b2Vec2{ center_in_meters.x - d_in_meters, center_in_meters.y - d_in_meters };
  aabb.upperBound = b2Vec2{ center_in_meters.x + d_in_meters, center_in_meters.y + d_in_meters };

  const auto& physics_c = get_first_component<SINGLE_Physics>(r);
  const b2Vec2 position = { center_in_meters.x, center_in_meters.y };

  struct QueryContext
  {
    std::unordered_map<entt::entity, std::vector<CollisionWithFixtureResult>> results;
    const std::function<bool(entt::registry&, entt::entity)>& cond;
    entt::registry& r;
    b2Vec2 position;
  };
  QueryContext context{ .cond = cond, .r = r, .position = position };

  const auto overlap_callback = [](const b2ShapeId shapeId, void* context) -> bool {
    QueryContext* queryContext = static_cast<QueryContext*>(context);

    const b2BodyId bodyId = b2Shape_GetBody(shapeId);
    const auto body_e = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyId)));
    const auto fixture_e = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(b2Shape_GetUserData(shapeId)));

    // Filter the fixtures
    if (queryContext->cond(queryContext->r, body_e)) {
      const b2Vec2 diff = b2Body_GetPosition(bodyId) - queryContext->position;

      CollisionWithFixtureResult res;
      res.d2 = diff.x * diff.x + diff.y * diff.y;
      res.fixture_e = fixture_e;
      if (!queryContext->results.contains(body_e))
        queryContext->results[body_e] = {};
      queryContext->results[body_e].push_back(res);
    }

    return true; // return true to continue the query
  };
  b2World_OverlapAABB(physics_c.worldId, aabb, b2DefaultQueryFilter(), overlap_callback, &context);

  return context.results;
};

float
meters_to_pixels(float meters)
{
  return meters * PIXELS_PER_METER;
};
glm::vec2
meters_to_pixels(b2Vec2 meters)
{
  return { meters.x * PIXELS_PER_METER, meters.y * PIXELS_PER_METER };
};

float
pixels_to_meters(float pixels)
{
  return pixels / PIXELS_PER_METER;
};
b2Vec2
pixels_to_meters(glm::vec2 pixels)
{
  auto p = pixels / PIXELS_PER_METER;
  return b2Vec2{ p.x, p.y };
};

/*
void
set_collision_filters(entt::registry& r, entt::entity e)
{
  // I am a ....
  constexpr uint16 FRIENDLY_UNIT_CATEGORY = 0x0001;
  constexpr uint16 FRIENDLY_BULLET_CATEGORY = 0x0002;
  constexpr uint16 ENEMY_UNIT_CATEGORY = 0x0004;
  constexpr uint16 ENEMY_BULLET_CATEGORY = 0x0008;
  constexpr uint16 WALL_CATEGORY = 0x0016;

  // I collide with ... (0xFFFF is everything)
  constexpr uint16_t FRIENDLY_UNIT_MASK = WALL_CATEGORY | ENEMY_BULLET_CATEGORY;
  constexpr uint16_t FRIENDLY_BULLET_MASK = WALL_CATEGORY | ENEMY_UNIT_CATEGORY;
  constexpr uint16_t ENEMY_UNIT_MASK = WALL_CATEGORY | FRIENDLY_BULLET_CATEGORY;
  constexpr uint16_t ENEMY_BULLET_MASK = WALL_CATEGORY | FRIENDLY_UNIT_CATEGORY;
  constexpr uint16_t WALL_MASK = 0xFFFF;

  const auto& physics_body = r.get<PhysicsBodyComponent>(e);
  const auto type = r.get<EntityTypeComponent>(e).type;
  const auto* team_c = r.try_get<TeamComponent>(e);

  return; // currently not doing this

  for (b2Fixture* fixture = physics_body.body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {

    b2Filter filter;

    if (type == EntityType::solid_wall) {
      filter.categoryBits = WALL_CATEGORY;
      filter.maskBits = WALL_MASK;
      fixture->SetFilterData(filter);
    }

    if (type == EntityType::bullet_default || type == EntityType::bullet_bouncy) {
      if (team_c->team == AvailableTeams::player) {
        filter.categoryBits = FRIENDLY_BULLET_CATEGORY;
        filter.maskBits = FRIENDLY_BULLET_MASK;
        fixture->SetFilterData(filter);
      } else if (team_c->team == AvailableTeams::enemy) {
        filter.categoryBits = ENEMY_BULLET_CATEGORY;
        filter.maskBits = ENEMY_BULLET_MASK;
        fixture->SetFilterData(filter);
      } else {
        SDL_Log("%s", std::format("warning; no team set on bullet. not setting physics masks");
      }
    }

    if (type == EntityType::actor_dungeon) {
      if (team_c->team == AvailableTeams::player) {
        filter.categoryBits = FRIENDLY_UNIT_CATEGORY;
        filter.maskBits = FRIENDLY_UNIT_MASK;
        fixture->SetFilterData(filter);
      } else if (team_c->team == AvailableTeams::enemy) {
        filter.categoryBits = ENEMY_UNIT_CATEGORY;
        filter.maskBits = ENEMY_UNIT_MASK;
        fixture->SetFilterData(filter);
      } else
        SDL_Log("%s", std::format("warning; no team set on actor_dungeon. not setting physics masks");
    }
  }
}
*/

} // namespace game2d