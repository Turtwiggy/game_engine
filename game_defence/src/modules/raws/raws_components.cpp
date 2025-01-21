#include "modules/raws/raws_components.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_breach_charge/breach_charge_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_move_player_on_map/move_player_on_map_components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

#include <box2d/b2_body.h>
#include <fstream>
#include <sstream>

namespace game2d {

template<class T>
auto
find_key_or_crash(const std::vector<T>& stuff, const std::string& key)
{
  const auto it = std::find_if(stuff.begin(), stuff.end(), [&key](const T& item) { return item.name == key; });
  if (it == stuff.end()) {
    SDL_Log("%s", std::format("unable to find key in std::vector<T>: {}", key).c_str());
    exit(1); // crash
  };
  return it;
};

Raws
load_raws(std::string path)
{
  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_with_comments = buffer.str();

  // remove comments from .jsonc file
  std::istringstream stream(data_with_comments);
  std::ostringstream output;
  std::string line;
  while (std::getline(stream, line)) {
    std::string cleaned_line = line.find("//") != std::string::npos ? "" : line;
    output << cleaned_line << "\n";
  }
  const std::string string_without_comments = output.str();

  nlohmann::json root = nlohmann::json::parse(string_without_comments);
  const auto raws = root.get<Raws>();
  return raws;
};

std::vector<std::string>
get_raws_keys(const Raws& raws)
{
  std::vector<std::string> results;

  for (const auto& key : raws.items)
    results.push_back(key.name);

  return results;
};

const Item
find_item(entt::registry& r, std::string key)
{
  const auto& rs = get_first_component<Raws>(r);
  const auto it = find_key_or_crash<Item>(rs.items, key);
  const auto idx = static_cast<int>(it - rs.items.begin());
  const auto& templ = rs.items[idx];
  return templ;
};

engine::SRGBColour
colour_tag_to_colour(const Raws& raws, const std::string& col)
{
  for (const auto& c : raws.colours)
    if (c.tag == col)
      return hex_to_srgb(c.hex);
  return engine::SRGBColour(1.0f, 1.0f, 1.0f, 1.0f);
};

entt::entity
create_transform(entt::registry& r, const std::string& name)
{
  const auto e = r.create();
  r.emplace<TagComponent>(e, name);
  r.emplace<WaitForInitComponent>(e);
  r.emplace<TransformComponent>(e);
  return e;
};

void
give_life(entt::registry& r, const entt::entity e, const glm::vec2& pos, const glm::vec2& size)
{
  const auto& raws = get_first_component<Raws>(r);
  const auto key = r.get<ItemKey>(e).key;
  const auto t = find_item(r, key);

  // const auto& name = r.get<NameComponent>(e).name;
  // SDL_Log("Give life: %s", name.c_str());

  // create_transform()
  {
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, t.renderable.sprite);

    // add a looping idle_anim
    if (t.renderable.sprite == "REF_IDLE") {
      SpriteAnimationState anim_c;
      anim_c.playing_animation_name = "REF_IDLE";
      anim_c.duration = 0.9f;

      // offset animation
      static engine::RandomState anim_rnd(0);
      anim_c.timer = engine::rand_det_s(anim_rnd.rng, 0.0f, anim_c.duration);

      r.emplace<SpriteAnimationState>(e, anim_c);
    }

    const auto col = colour_tag_to_colour(raws, t.renderable.colour);
    r.emplace<DefaultColour>(e, col);
    set_colour(r, e, col);

    TransformComponent tf;
    tf.position = { pos.x, pos.y, 0.0f };
    tf.scale = { size.x, size.y, 0.0f };
    r.emplace<TransformComponent>(e, tf);

    set_z_index(r, e, ZLayer::DEFAULT);
  }

  // create_physics()
  if (t.phys_body.has_value()) {
    const auto& my_body_def = t.phys_body.value();
    auto is_bullet = my_body_def.is_bullet;
    auto is_static = my_body_def.is_static;

    // Create a physics body.
    //
    auto& physics_c = get_first_component<SINGLE_Physics>(r);

    // Bodies are built using the following steps:
    // Define a body with position, damping, etc.
    // Use the world object to create the body.
    // Define fixtures with a shape, friction, density, etc.
    // Create fixtures on the body.

    b2BodyDef body_def;
    body_def.position.Set(pos.x, pos.y);
    body_def.angle = 0.0f;
    body_def.fixedRotation = true;
    body_def.bullet = is_bullet;
    body_def.type = is_static ? b2_staticBody : b2_dynamicBody;
    body_def.linearVelocity = b2Vec2_zero;
    body_def.linearDamping = my_body_def.linear_damping;
    body_def.angularDamping = my_body_def.angular_damping;

    // box2d: create body
    b2Body* body = nullptr;
    body = physics_c.world->CreateBody(&body_def);
    // SDL_Log("creating physics body..");

    // box2d: give link to entt
    body->GetUserData().pointer = (uintptr_t)e;

    // entt: create body representation
    auto& body_c = r.emplace<PhysicsBodyComponent>(e, PhysicsBodyComponent{ body });

    if (!t.phys_fixtures.has_value()) {
      SDL_Log("(Error) phys_body defined, but not phys_fixtures");
      exit(1);
    }

    if (t.phys_fixtures.has_value()) {
      const auto& fixtures = t.phys_fixtures.value();

      if (fixtures.size() == 0) {
        SDL_Log("(Error) phys_fixtures size 0 when phys_body defined");
        exit(1);
      }

      for (const auto& fix : fixtures) {
        auto tag = fix.tag;
        auto type = fix.type;
        auto is_sensor = fix.is_sensor;
        auto density = fix.density;
        auto friction = fix.friction;
        auto restitution = fix.restitution;

        b2FixtureDef fixture_def;
        fixture_def.friction = friction;
        fixture_def.density = density;
        fixture_def.restitution = restitution;
        fixture_def.isSensor = is_sensor;

        b2Fixture* fixture = nullptr;

        if (type == "circle") {
          b2CircleShape circle;
          circle.m_radius = fix.radius;
          fixture_def.shape = &circle;
          fixture = body->CreateFixture(&fixture_def);
          // SDL_Log("creating circle fixture..");
        }

        if (type == "box") {
          b2PolygonShape box;
          box.SetAsBox(size.x / 2.0f, size.y / 2.0f);
          fixture_def.shape = &box;
          fixture = body->CreateFixture(&fixture_def);
          // SDL_Log("creating box fixture..");
        }

        if (fixture == nullptr) {
          SDL_Log("(Error) unknown fixture type: %s", type.c_str());
          exit(1);
        }

        // entt: create fixture representation
        PhysicsFixtureComponent fixture_c;
        fixture_c.body = body;
        fixture_c.fixture = fixture;
        auto fixture_e = create_empty<PhysicsFixtureComponent>(r);
        r.emplace_or_replace<TagComponent>(fixture_e, fix.tag);
        r.emplace<HasParentComponent>(fixture_e, e); // link fixture => body
        body_c.fixtures.push_back(fixture_e);        // link body => fixture

        // box2d: give link to entt
        fixture->GetUserData().pointer = (uint32)fixture_e;
      }
    }

    // While we're creating it, update the transform
    auto& transform_c = r.get<TransformComponent>(e);
    transform_c.scale.x = size.x;
    transform_c.scale.y = size.y;
  }

  // r.emplace<DefaultSizeComponent>(e, size);
};

void
remove_life(entt::registry& r, const entt::entity e)
{
  r.remove<TransformComponent>(e);
  r.remove<SpriteComponent>(e);

  if (auto* timer_c = r.try_get<EntityTimedLifecycle>(e))
    r.remove<EntityTimedLifecycle>(e);

  if (auto* callback_c = r.try_get<OnDeathCallbacks>(e))
    r.remove<OnDeathCallbacks>(e);

  if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {
    auto& physics_c = get_first_component<SINGLE_Physics>(r);
    physics_c.world->DestroyBody(pb->body);
    r.remove<PhysicsBodyComponent>(e);
  }
};

entt::entity
spawn(entt::registry& r, const std::string& key)
{
  const auto& rs = get_first_component<Raws>(r);
  const auto it = find_key_or_crash<Item>(rs.items, key);
  const auto idx = static_cast<int>(it - rs.items.begin());
  const Item& item_template = rs.items[idx];

  const auto e = r.create();
  r.emplace<TagComponent>(e, item_template.name);
  r.emplace<WaitForInitComponent>(e);
  r.emplace<OnDeathCallbacks>(e);
  r.emplace<ItemKey>(e, key);

  // item component is used by the inventory ui
  UI_ItemComponent item_c;
  item_c.display_icon = item_template.renderable.sprite;
  item_c.display_name = item_template.display_name;
  item_c.display_desc = item_template.display_desc;
  item_c.parent_slot = entt::null; // set outside this func
  r.emplace<UI_ItemComponent>(e, item_c);

  if (item_template.use.has_value())
    r.emplace<Use>(e, item_template.use.value());

  if (item_template.inventory.has_value())
    r.emplace<DefaultInventory>(e, DefaultInventory(r, item_template.inventory.value().size));

  if (item_template.stats.has_value())
    r.emplace<HealthComponent>(e, item_template.stats->hp, item_template.stats->max_hp);
  r.emplace<DefenceComponent>(e, 0); // should be determined by equipment
  r.emplace<InputComponent>(e);

  if (item_template.move_speed.has_value()) {
    LimitMovementComponent move_c;
    move_c.path_size = item_template.move_speed->speed;
    r.emplace<LimitMovementComponent>(e, move_c);
  }
  r.emplace<PathfindComponent>(e, -1, true);
  r.emplace<SpawnParticlesOnDeath>(e);

  // Give each mob a random name
  static engine::RandomState rnd(0);
  const auto& names = get_first_component<SINGLE_NamesComponent>(r);
  r.emplace<NameComponent>(e, names.name[engine::rand_det_s(rnd.rng, 0, int(names.name.size()))]);

  // todo: replace with item traits
  if (key == "breach_charge") {
    add_bomb_callback(r, e);
    r.emplace<EntityTimedLifecycle>(e, 3 * 1000);
  }

  if (item_template.traits.has_value())
    for (const auto& trait : item_template.traits.value())
      SDL_Log("item with trait: %s", trait.key.c_str());

  return e;
};

entt::entity
spawn_particle_emitter(entt::registry& r, const std::string& key, const glm::vec2& pos, const entt::entity parent)
{
  const auto e = create_transform(r, "particle_emitter");

  r.emplace<SetPositionAtDynamicTarget>(e);
  r.emplace<DynamicTargetComponent>(e, parent);

  Particle pdesc;
  pdesc.time_to_live_ms = 1 * 1000;
  pdesc.start_size = 6;
  pdesc.end_size = 2;
  if (key.find("default_explode") != std::string::npos) {
    pdesc.start_size = 16;
    pdesc.end_size = 4;
  }

  // which particle to spawn?
  ParticleEmitterComponent pedesc;
  pedesc.particle_to_emit = pdesc;
  if (key.find("default_explode") != std::string::npos) {
    pedesc.expires = true;
    pedesc.particles_to_spawn_before_emitter_expires = 10;
    pedesc.random_velocity = true;
    pedesc.spawn_all_particles_at_once = true;
  }
  r.emplace<ParticleEmitterComponent>(e, pedesc);

  // emit: particles
  CooldownComponent cooldown;
  cooldown.time_max = 0.1f;
  cooldown.time = cooldown.time_max;
  r.emplace<CooldownComponent>(e, cooldown);

  set_size(r, e, { 0, 0 }); // no size for particle emitter
  return e;
};

entt::entity
spawn_particle(entt::registry& r, const std::string& key, const Particle& desc)
{
  const auto e = create_transform(r, "particle");
  r.emplace<SpriteComponent>(e);
  set_sprite(r, e, "EMPTY");

  r.emplace<EntityTimedLifecycle>(e, desc.time_to_live_ms);
  r.emplace<VelocityComponent>(e, VelocityComponent{ desc.velocity.x, desc.velocity.y });

  // WARNING: random rotation here
  static engine::RandomState rnd(0);
  auto& t = r.get<TransformComponent>(e);
  t.rotation_radians.z = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI); // rnd rotation

  // make it shrink
  ScaleOverTimeComponent sotc;
  sotc.seconds_until_complete = desc.time_to_live_ms / 1000.0f;
  sotc.start_size = desc.start_size;
  sotc.end_size = desc.end_size;
  r.emplace<ScaleOverTimeComponent>(e, sotc);

  if (auto* col = r.try_get<DefaultColour>(e))
    set_colour(r, e, col->colour);

  set_position(r, e, desc.position);
  return e;
};

} // namespace game2d