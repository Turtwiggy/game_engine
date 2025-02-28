#include "modules/core_raws/raws_components.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_enemy_charger/enemy_charger_components.hpp"
#include "modules/actor_exploder/actor_exploder_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/core_colour/components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/core_renderer/helpers.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/system_alpha_based_on_lifecycle/alpha_based_on_lifecycle_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "raws_helpers.hpp"

#include <box2d/b2_body.h>
#include <box2d/b2_dynamic_tree.h>
#include <box2d/b2_fixture.h>
#include <glm/fwd.hpp>
#include <magic_enum.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace game2d {

template<class T>
auto
find_key_or_crash(const std::vector<T>& stuff, const std::string& key)
{
  const auto it = std::find_if(stuff.begin(), stuff.end(), [&key](const T& item) { return item.name == key; });
  if (it == stuff.end()) {
    throw std::runtime_error(std::format("unable to find key in std::vector<T>: {}", key).c_str());
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
    const auto clean_line = remove_comment(line);
    output << clean_line << "\n";
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
  return { 1.0f, 1.0f, 1.0f, 1.0f };
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

b2Fixture*
create_fixture(b2Body* body, const PhysicsFixtureDef& fix, const b2Vec2 size_in_meters)
{
  auto tag = fix.tag;
  auto type = fix.type;
  auto is_sensor = fix.is_sensor;
  auto density = fix.density;
  auto friction = fix.friction;
  auto restitution = fix.restitution;

  b2Vec2 offset{ 0, 0 };
  if (fix.offset.size() > 0)
    offset = pixels_to_meters({ fix.offset[0].x, fix.offset[0].y });

  b2Vec2 size = size_in_meters;
  if (fix.size.size() > 0)
    size = pixels_to_meters({ fix.size[0].x, fix.size[0].y });

  b2FixtureDef fixture_def;
  fixture_def.friction = friction;
  fixture_def.density = density;
  fixture_def.restitution = restitution;
  fixture_def.isSensor = is_sensor;

  b2Fixture* fixture = nullptr;

  if (type == "circle") {
    b2CircleShape circle;
    circle.m_radius = size.x;
    circle.m_p.Set(offset.x, offset.y);
    fixture_def.shape = &circle;
    fixture = body->CreateFixture(&fixture_def);
    // SDL_Log("creating circle fixture..");
  }

  if (type == "box") {
    b2PolygonShape box;
    box.SetAsBox(size.x / 2.0f, size.y / 2.0f, offset, 0.0f);
    fixture_def.shape = &box;
    fixture = body->CreateFixture(&fixture_def);
    // SDL_Log("creating box fixture..");
  }

  if (fixture == nullptr) {
    SDL_Log("(Error) unknown fixture type: %s", type.c_str());
    exit(1);
  }

  return fixture;
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
    const auto col = colour_tag_to_colour(raws, t.renderable.colour);

    r.emplace<SpriteComponent>(e);
    r.emplace<DefaultColour>(e, col);
    r.emplace<TransformComponent>(e,
                                  TransformComponent{
                                    .position = { pos.x, pos.y, 0.0f },
                                    .scale = { size.x, size.y, 0.0f },
                                  });

    set_colour(r, e, col);
    set_sprite(r, e, t.renderable.sprite);
    set_z_index(r, e, ZLayer::DEFAULT);
  }

  // create_physics()
  if (t.phys_body.has_value()) {
    const auto& my_body_def = t.phys_body.value();
    const auto is_bullet = my_body_def.is_bullet;
    const auto is_static = my_body_def.is_static;

    // Create a physics body.
    //
    const auto& physics_c = get_first_component<SINGLE_Physics>(r);

    // Bodies are built using the following steps:
    // Define a body with position, damping, etc.
    // Use the world object to create the body.
    // Define fixtures with a shape, friction, density, etc.
    // Create fixtures on the body.

    const b2Vec2 pos_in_meters = pixels_to_meters(pos);
    const b2Vec2 size_in_meters = pixels_to_meters(size);

    b2BodyDef body_def;
    body_def.position.Set(pos_in_meters.x, pos_in_meters.y);
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

      for (const PhysicsFixtureDef& data : fixtures) {

        auto* fixture = create_fixture(body, data, size_in_meters);
        // SDL_Log("Created fixture... %s", fix.tag.c_str());

        // entt: create fixture representation
        PhysicsFixtureComponent fixture_c;
        fixture_c.body = body;
        fixture_c.fixture = fixture;
        auto fixture_e = create_empty<PhysicsFixtureComponent>(r, fixture_c);
        r.emplace_or_replace<TagComponent>(fixture_e, data.tag);
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

  // add_traits()
  {
    std::vector<TraitOnDisk> traits;
    if (t.traits.has_value())
      traits = t.traits.value();

    // Store traits on a per-entity basis as well
    TraitComponent trait_c;
    for (const auto& t : traits) {
      const AquirableTrait typed_t = magic_enum::enum_cast<AquirableTrait>(t.key).value();
      trait_c.traits.emplace(typed_t);
    }
    r.emplace<TraitComponent>(e, trait_c);

    bool big_explode = false;

    for (const auto& trait_str : traits) {
      const auto trait_enum = magic_enum::enum_cast<AquirableTrait>(trait_str.key).value();

      if (trait_enum == AquirableTrait::DIRECT) {
        ApplyForceToDynamicTarget tgt_c;
        tgt_c.orbit = false;
        tgt_c.reduce_thrusters = false;
        tgt_c.speed = 1.0f; // m/s
        r.emplace<ApplyForceToDynamicTarget>(e, tgt_c);
        r.get<PhysicsBodyComponent>(e).body->SetLinearDamping(1.0);
      }

      if (trait_enum == AquirableTrait::ARC_ANGLE) {
        // generate a random angle to approach from

#if defined(_DEBUG)
        // static engine::RandomState angle_rnd(0); // same roll every time
        static engine::RandomState angle_rnd(engine::get_system_time_for_seed());
#else
        static engine::RandomState angle_rnd(engine::get_system_time_for_seed());
#endif

        const float angle = engine::rand_det_s(angle_rnd.rng, 0.0f, engine::TWO_PI);
        r.emplace<ApplyForceToApproachTargetFromAngle>(e, angle);
        r.get<PhysicsBodyComponent>(e).body->SetLinearDamping(1.0);

        // const auto angle_str = std::format("Creating enemy... approaching from {}", angle);
        // SDL_Log("%s", angle_str.c_str());
      }

      if (trait_enum == AquirableTrait::KEEP_DISTANCE) {
        ApplyForceToDynamicTarget tgt_c;
        tgt_c.orbit = true;
        tgt_c.reduce_thrusters = true;
        tgt_c.speed = 1.0f;                    // m/s
        tgt_c.distance_to_reduce_thrust = 6.0; // meters to shoot from
        r.emplace<ApplyForceToDynamicTarget>(e, tgt_c);
        r.get<PhysicsBodyComponent>(e).body->SetLinearDamping(1.0);
      }

      if (trait_enum == AquirableTrait::PROJECTILE) {
        r.emplace<ProjectileEnemyComponent>(e);

        // TODO: give the enemy a weapon, dont attach these components to enemy?
        r.emplace<CooldownComponent>(e, CooldownComponent{ 2.0f, 0.0 });
        r.emplace<BulletDamage>(e, BulletDamage{ 1 });
        r.emplace<BulletPierce>(e, BulletPierce{ 1 });
        r.emplace<BulletSpeed>(e, BulletSpeed{ 1.0f });
        r.emplace<BulletSize>(e, BulletSize{ { 18, 18 } });
        // r.emplace<BulletKnockback>(e, BulletKnockback{ 50 });
        // r.emplace<WeaponSpread>(wep_e);
        // r.emplace<WeaponProjectiles>(wep_e, 1);
      }

      if (trait_enum == AquirableTrait::EXPLODE) {
        add_explode_on_death_callback(r, e);
        big_explode = true;
      }

      if (trait_enum == AquirableTrait::CHARGE) {
        r.emplace<ChargerEnemyComponent>(e);
        r.emplace<CooldownComponent>(e);
      }
    }

    // Spawn particles on death
    if (!big_explode) {
      auto& callbacks_c = r.get<OnDeathCallbacks>(e);
      const auto spawn_particles_callback = [](entt::registry& r, entt::entity e) {
        RequestToSpawnParticles request;
        request.key = "enemy_death";
        request.position = get_position(r, e);
        create_empty<RequestToSpawnParticles>(r, request);
      };
      callbacks_c.callbacks.push_back(spawn_particles_callback);
    }
  }

  r.emplace<DefaultSizeComponent>(e, size);
};

void
remove_life(entt::registry& r, const entt::entity e)
{
  r.remove<TransformComponent>(e);
  r.remove<SpriteComponent>(e);

  if (auto* timer_c = r.try_get<EntityTimedLifecycle>(e))
    r.remove<EntityTimedLifecycle>(e);

  if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {
    const auto& physics_c = get_first_component<SINGLE_Physics>(r);
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
  const auto& templ = rs.items[idx];

  const auto e = r.create();
  r.emplace<TagComponent>(e, templ.name);
  r.emplace<WaitForInitComponent>(e);
  r.emplace<OnDeathCallbacks>(e);
  r.emplace<ItemKey>(e, key);
  // r.emplace<Item>(e, templ);

  r.emplace<InputComponent>(e);

  return e;
};

entt::entity
spawn_particle_emitter(entt::registry& r, const std::string& key, const entt::entity parent)
{
  const auto e = create_transform(r, "particle_emitter");

  r.emplace<SetPositionAtDynamicTarget>(e);
  r.emplace<DynamicTargetComponent>(e, parent);

  Particle pdesc;
  pdesc.time_to_live_ms = 1 * 1000;
  pdesc.start_size = { 6, 6 };
  pdesc.end_size = { 2, 2 };
  if (key.find("default_explode") != std::string::npos) {
    pdesc.start_size = { 16, 16 };
    pdesc.end_size = { 4, 4 };
  }
  if (key.find("enemy_death") != std::string::npos) {
    pdesc.start_size = { 16, 16 };
    pdesc.end_size = { 0, 0 };
    pdesc.start_colour = hex_to_srgb("#a64a2e"); // dark red
  }
  if (key.find("death_exploder") != std::string::npos) {
    pdesc.start_size = { explosion_radius_pixels * 2, explosion_radius_pixels * 2 };
    pdesc.end_size = { explosion_radius_pixels * 1, explosion_radius_pixels * 1 };
  }
  if (key.find("default_trail") != std::string::npos) {
    pdesc.start_size = { 2, 2 };
    pdesc.end_size = { 0, 0 };
    pdesc.time_to_live_ms = static_cast<int>(0.5 * 1000);
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
  if (key.find("enemy_death") != std::string::npos) {
    pedesc.expires = true;
    pedesc.particles_to_spawn_before_emitter_expires = 6;
    pedesc.random_velocity = true;
    pedesc.spawn_all_particles_at_once = true;
  }
  if (key.find("death_exploder") != std::string::npos) {
    pedesc.expires = true;
    pedesc.particles_to_spawn_before_emitter_expires = 10;
    pedesc.random_velocity = true;
    pedesc.spawn_all_particles_at_once = true;
  }
  r.emplace<ParticleEmitterComponent>(e, pedesc);

  // emit: particles
  CooldownComponent cooldown;
  cooldown.time_max = 0.1f;

  if (key.find("default_trail") != std::string::npos) {
    cooldown.time_max = 1 / 30.0f; // 30 particle-a-second innit
  }

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

  if (desc.fade)
    r.emplace<SetAlphaBasedOnLifecycleComponent>(e);

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

  // if (auto* col = r.try_get<DefaultColour>(e))
  //   set_colour(r, e, col->colour);
  set_colour(r, e, desc.start_colour);

  set_position(r, e, desc.position);
  return e;
};

// entt::entity
// spawn_floor(entt::registry& r, const std::string& key, const glm::vec2& pos, const glm::vec2& size)
// {
//   const auto floor_e = create_transform(r, "floor");
//   r.emplace<DefaultColour>(floor_e, engine::SRGBColour{ 0.5f, 0.5f, 0.5f, 1.0f });
//   r.emplace<SpriteComponent>(floor_e);
//   set_sprite(r, floor_e, "EMPTY");
//   set_position(r, floor_e, pos);
//   set_size(r, floor_e, size);
//   set_colour(r, floor_e, r.get<DefaultColour>(floor_e).colour);
//   set_z_index(r, floor_e, ZLayer::FLOOR);
//   r.emplace<FloorComponent>(floor_e);
//   return floor_e;
// };

} // namespace game2d