#include "engine/physics/physics_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core_collisions/resolve_collisions_helpers.hpp"

#include <SDL2/SDL_log.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_world_callbacks.h>
#include <format>
#include <functional>
#include <limits>
#include <stdexcept>

namespace game2d {

void
emplace_or_replace_physics_world(entt::registry& r)
{
  // store one physics world...
  static b2World* world = new b2World(b2Vec2(0.0f, 0.0f));
  static PhysicsEvents* listener = new PhysicsEvents(r);

  // Add callback
  static bool first_time = true;
  if (first_time) {
    world->SetContactListener(listener);
    first_time = false;
  }

  // cleanup physics world...
  static bool needs_deleting = false;
  if (needs_deleting) {
    SDL_Log("%s", std::format("cleaning up physics world..").c_str());

    b2Joint* joint = world->GetJointList();
    while (joint) {
      b2Joint* j = joint;
      joint = joint->GetNext();
      world->DestroyJoint(j);
    }

    b2Body* body = world->GetBodyList();
    while (body) {
      b2Body* b = body;
      body = body->GetNext();
      world->DestroyBody(b);
    }
  }

  needs_deleting = true;
  SDL_Log("%s", std::format("physics world set to clean up... (deleted: {})", needs_deleting).c_str());

  destroy_first_and_create<SINGLE_Physics>(r, SINGLE_Physics{ world });
  destroy_first_and_create<SINGLE_PhysicsEvents>(r, SINGLE_PhysicsEvents{ listener });
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
  const auto& body_c = r.get<PhysicsBodyComponent>(e);
  for (const entt::entity fix_e : body_c.fixtures) {
    const auto& fix_c = r.get<PhysicsFixtureComponent>(fix_e);
    const auto* b2_fixture = fix_c.fixture;
    const auto& fix_tag = r.get<TagComponent>(fix_e);
    if (fix_tag.tag == tag)
      return fix_e;
  }

  SDL_Log("missing get_fixture_by_tag(): %s", tag.c_str());
  throw std::runtime_error("missing get_fixture_by_tag()");
  exit(1); // explode

  return entt::null;
};

class SearchAreaCallback : public b2QueryCallback
{
public:
  float nearest_distance_squared = std::numeric_limits<float>::max();
  std::vector<entt::entity> results;

  // Something hit the matching criteria
  bool ReportFixture(b2Fixture* fixture) override
  {
    auto* body = fixture->GetBody();
    auto body_e = (entt::entity)body->GetUserData().pointer;
    results.push_back(body_e);
    return true; // keep going to find all fixtures in query area
  };
};

class FilteredSearchAreaCallback : public b2QueryCallback
{
public:
  float nearest_distance_squared = std::numeric_limits<float>::max();
  std::vector<std::pair<int, entt::entity>> results; // distance to the entity

  b2Vec2 position;
  entt::registry& r;
  const std::function<bool(entt::registry&, entt::entity)>& cond;

  FilteredSearchAreaCallback(entt::registry& r,
                             const b2Vec2& center,
                             const std::function<bool(entt::registry&, entt::entity)>& cond)
    : position(center)
    , r(r)
    , cond(cond) {};

  // Something hit the matching criteria
  bool ReportFixture(b2Fixture* fixture) override
  {
    auto* body = fixture->GetBody();
    auto body_e = (entt::entity)body->GetUserData().pointer;

    // Filter the fixtures
    if (cond(r, body_e)) {
      const b2Vec2 diff = body->GetPosition() - position;
      const float d2 = diff.LengthSquared();
      results.push_back({ d2, body_e });
    }

    return true; // keep going to find all fixtures in query area
  };
};

std::vector<entt::entity>
get_all_in_area(entt::registry& r, glm::vec2 center, float d)
{
  SearchAreaCallback callback;
  b2AABB aabb;
  aabb.lowerBound = b2Vec2{ center.x - d, center.y - d };
  aabb.upperBound = b2Vec2{ center.x + d, center.y + d };

  const auto& physics_c = get_first_component<SINGLE_Physics>(r);
  physics_c.world->QueryAABB(&callback, aabb);

  return callback.results;
};

std::vector<std::pair<int, entt::entity>>
get_all_in_area_filtered(entt::registry& r,
                         const glm::vec2 center,
                         const float d,
                         const std::function<bool(entt::registry&, entt::entity)>& cond)
{
  FilteredSearchAreaCallback callback(r, { center.x, center.y }, cond);

  b2AABB aabb;
  aabb.lowerBound = b2Vec2{ center.x - d, center.y - d };
  aabb.upperBound = b2Vec2{ center.x + d, center.y + d };

  const auto& physics_c = get_first_component<SINGLE_Physics>(r);
  physics_c.world->QueryAABB(&callback, aabb);

  return callback.results;
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