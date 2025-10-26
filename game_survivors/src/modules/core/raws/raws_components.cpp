#include "pch.hpp"

#include "modules/core/raws/raws_components.hpp"

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
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_enemy_charger/enemy_charger_components.hpp"
#include "modules/actors/actor_enemy_flow/enemy_flow_components.hpp"
#include "modules/actors/actor_exploder/actor_exploder_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_components.hpp"
#include "modules/combat/combat_weapon_type_projectile/combat_weapon_type_projectile_components.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/systems/system_alpha_based_on_lifecycle/alpha_based_on_lifecycle_components.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_move_to_target_via_lerp/components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"
#include "modules/systems/system_traits/trait_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_debug_effects/effects_helpers.hpp"
#include "raws_helpers.hpp"

namespace game2d {

template<class T>
auto
find_key_or_crash(const std::vector<T>& stuff, const std::string& key)
{
  const auto it = std::find_if(stuff.begin(), stuff.end(), [&key](const T& item) { return item.name == key; });
  if (it == stuff.end()) {
    auto err = std::format("unable to find key in std::vector<T>: {}", key);
    SDL_Log("%s", err.c_str());
    throw std::runtime_error(err.c_str());
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

b2ShapeId
create_fixture(b2BodyId bodyId, const PhysicsFixtureDef& fix, const b2Vec2 size_in_meters)
{
  const auto tag = fix.tag;
  const auto type = fix.type;
  const auto is_sensor = fix.is_sensor;
  const auto density = fix.density;
  const auto friction = fix.friction;

  b2Vec2 offset{ 0, 0 };
  if (!fix.offset.empty())
    offset = pixels_to_meters({ fix.offset[0].x, fix.offset[0].y });

  b2Vec2 size = size_in_meters;
  if (!fix.size_in_pixels.empty())
    size = pixels_to_meters({ fix.size_in_pixels[0].x, fix.size_in_pixels[0].y });

  b2ShapeDef shape_def = b2DefaultShapeDef();
  shape_def.density = density;
  shape_def.isSensor = is_sensor;
  shape_def.enableSensorEvents = true;
  shape_def.enableContactEvents = true;

  // note: looks like box2d 3.1+ makes use of materials
  // shape_def.material
  // shape_def.friction = friction;

  if (type == "box") {
    b2Polygon box = b2MakeBox(size.x * 0.5f, size.y * 0.5f);

    for (int i = 0; i < box.count; i++)
      box.vertices[i] += offset;
    box.centroid += offset;

    const b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shape_def, &box);
    return shapeId;
  }

  else if (type == "circle") {
    const b2Circle circle = { { offset }, size.x * 0.5f };
    const b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shape_def, &circle);
    return shapeId;
  }

  SDL_Log("(Error) unknown fixture type: %s", type.c_str());
  exit(1);
  return {};
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
    const auto& physics_c = get_first_component<SINGLE_Physics>(r);
    const auto& my_body_def = t.phys_body.value();
    const auto is_bullet = my_body_def.is_bullet;
    const auto is_static = my_body_def.is_static;

    // Bodies are built using the following steps:
    // Define a body with position, damping, etc.
    // Use the world object to create the body.
    // Define fixtures with a shape, friction, density, etc.
    // Create fixtures on the body.

    const b2Vec2 pos_in_meters = pixels_to_meters(pos);
    const b2Vec2 size_in_meters = pixels_to_meters(size);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = is_static ? b2_staticBody : b2_dynamicBody;
    bodyDef.position = pos_in_meters;
    bodyDef.rotation = b2Rot_identity;
    bodyDef.fixedRotation = true;
    bodyDef.isBullet = is_bullet;
    bodyDef.linearVelocity = b2Vec2_zero;
    bodyDef.linearDamping = my_body_def.linear_damping;
    bodyDef.angularDamping = my_body_def.angular_damping;
    bodyDef.userData = (void*)static_cast<uintptr_t>(entt::to_integral(e));

    // box2d: create body
    b2BodyId bodyId = b2CreateBody(physics_c.worldId, &bodyDef);

    // entt: create body representation
    auto& body_c = r.emplace<PhysicsBodyComponent>(e, PhysicsBodyComponent{ .bodyId = bodyId });

    if (!t.phys_fixtures.has_value()) {
      SDL_Log("(Error) phys_body defined, but not phys_fixtures");
      exit(1);
    }

    if (t.phys_fixtures.has_value()) {
      const auto& fixtures = t.phys_fixtures.value();

      if (fixtures.empty()) {
        SDL_Log("(Error) phys_fixtures size 0 when phys_body defined");
        exit(1);
      }

      for (const PhysicsFixtureDef& data : fixtures) {

        const auto shapeId = create_fixture(bodyId, data, size_in_meters);
        // SDL_Log("Created fixture... %s", fix.tag.c_str());

        // entt: create fixture representation
        PhysicsFixtureComponent fixture_c;
        fixture_c.bodyId = bodyId;
        fixture_c.shapeId = shapeId;
        const auto fixture_e = create_empty<PhysicsFixtureComponent>(r, fixture_c);
        r.emplace_or_replace<TagComponent>(fixture_e, TagComponent{ data.tag });
        r.emplace<ItemKey>(fixture_e, ItemKey{ key });
        r.emplace<HasParentComponent>(fixture_e, e); // link fixture => body
        body_c.fixtures.push_back(fixture_e);        // link body => fixture

        // box2d: give link to entt
        b2Shape_SetUserData(shapeId, (void*)static_cast<uintptr_t>(entt::to_integral(fixture_e)));
      }
    }
  }

  // add_traits()
  {
    std::vector<KeyValueOnDisk> traits;
    if (t.traits.has_value())
      traits = t.traits.value();

    // Store traits on a per-entity basis as well
    AiBehavioursComponent trait_c;
    for (const auto& tr : traits) {
      const AiBehaviour typed_t = magic_enum::enum_cast<AiBehaviour>(tr.key).value();
      trait_c.traits.emplace(typed_t);
    }
    if (!traits.empty())
      r.emplace<AiBehavioursComponent>(e, trait_c);

    for (const auto& trait_str : traits) {
      const auto trait_enum = magic_enum::enum_cast<AiBehaviour>(trait_str.key).value();

      if (trait_enum == AiBehaviour::DIRECT) {
        ApplyForceToDynamicTarget tgt_c;
        tgt_c.orbit = false;
        tgt_c.reduce_thrusters = false;
        r.emplace<ApplyForceToDynamicTarget>(e, tgt_c);
        b2Body_SetLinearDamping(r.get<PhysicsBodyComponent>(e).bodyId, 1.0f);
      }

      if (trait_enum == AiBehaviour::ARC_ANGLE) {
        // generate a random angle to approach from

#if defined(_DEBUG)
        // static engine::RandomState angle_rnd(0); // same roll every time
        static engine::RandomState angle_rnd(engine::get_system_time_for_seed());
#else
        static engine::RandomState angle_rnd(engine::get_system_time_for_seed());
#endif

        const float angle = engine::rand_det_s(angle_rnd.rng, 0.0f, engine::TWO_PI);
        r.emplace<ApplyForceToApproachTargetFromAngle>(e, angle);
        b2Body_SetLinearDamping(r.get<PhysicsBodyComponent>(e).bodyId, 1.0f);

        // const auto angle_str = std::format("Creating enemy... approaching from {}", angle);
        // SDL_Log("%s", angle_str.c_str());
      }

      if (trait_enum == AiBehaviour::KEEP_DISTANCE) {
        ApplyForceToDynamicTarget tgt_c;
        tgt_c.orbit = true;
        tgt_c.reduce_thrusters = true;
        tgt_c.distance_to_reduce_thrust = 6.0; // meters to shoot from
        r.emplace<ApplyForceToDynamicTarget>(e, tgt_c);
        b2Body_SetLinearDamping(r.get<PhysicsBodyComponent>(e).bodyId, 1.0f);
      }

      if (trait_enum == AiBehaviour::PROJECTILE) {
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

      if (trait_enum == AiBehaviour::EXPLODE) {

        const std::function<bool(entt::registry&, entt::entity)> filter_criteria = [](entt::registry& r,
                                                                                      entt::entity e) -> bool {
          bool valid_target = false;
          valid_target |= r.try_get<EnemyComponent>(e) != nullptr;
          valid_target |= r.try_get<PlayerBoatComponent>(e) != nullptr;
          return valid_target;
        };

        const float enemy_explosion_radius_pixels = 50.0f;
        add_explode_on_death_callback(r, e, enemy_explosion_radius_pixels, filter_criteria, "death_exploder");
      }

      if (trait_enum == AiBehaviour::CHARGE) {
        r.emplace<ChargerEnemyComponent>(e);
        r.emplace<CooldownComponent>(e);
      }

      if (trait_enum == AiBehaviour::FLOW) {
        r.emplace<FlowEnemyComponent>(e);
      }
    }

    // Spawn particles on death
    if (auto* callbacks_c = r.try_get<OnDeathCallbacks>(e)) {
      const auto spawn_particles_callback = [](entt::registry& r, entt::entity e) {
        RequestToSpawnParticles request;
        request.key = "enemy_death";
        request.position = get_position(r, e);
        create_empty<RequestToSpawnParticles>(r, request);
      };
      callbacks_c->callbacks.push_back(spawn_particles_callback);
    }
  }

  r.emplace<ActorSpeedComponent>(e, ActorSpeedComponent{ t.speed, t.speed });
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
    b2DestroyBody(pb->bodyId);
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
spawn_particle_emitter(entt::registry& r, const RequestToSpawnParticles& req)
{
  const auto parent = req.parent;
  if (parent == entt::null)
    throw std::runtime_error("particle-emitter parent not set.");

  const auto key = req.key;

  // trial: new vfx for exploder

  // note: * 3.0f as it should be 2.0f (as width = 2.0 * radius),
  // but the explosions sprite doesnt fill up the entire width, so scale it up.
  if (key.find("death_exploder") != std::string::npos)
    return spawn_fx(r, "EXPLODE_FX_6", req.position, { req.radius_pixels_upper * 3.0f, req.radius_pixels_upper * 3.0f });
  // if (key.find("death_sea_mine") != std::string::npos)
  // return spawn_fx(r, "S6_EXPLODE_FX_7", req.position, { req.radius_pixels * 2.0f, req.radius_pixels * 2.0f });

  const auto e = create_transform(r, "particle_emitter");
  r.emplace<SetPositionAtDynamicTarget>(e);
  r.emplace<DynamicTargetComponent>(e, parent);

  Particle pdesc;
  pdesc.time_to_live_ms = 1 * 1000;

  if (key.find("default_trail") != std::string::npos) {
    pdesc.size_curve = { { 2, 2 }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(0.5 * 1000);
    if (req.colour.has_value())
      pdesc.start_colour = req.colour.value();
  }
  //
  else if (key.find("default_explode") != std::string::npos) {
    pdesc.size_curve = { { 16, 16 }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(1.0 * 1000);
    pdesc.random_velocity_bound = 5;
  }
  //
  else if (key.find("death_sea_mine") != std::string::npos) {
    pdesc.size_curve = { { 128, 128 }, { 0, 0 } };
    pdesc.start_colour = hex_to_srgb("#f15f22"); // explosion colour
    pdesc.time_to_live_ms = static_cast<int>(0.8f * 1000);
    pdesc.random_radius_bound_lower = -req.radius_pixels_upper;
    pdesc.random_radius_bound_upper = req.radius_pixels_upper;
    pdesc.make_darker_based_on_distance_from_center = true;
    pdesc.random_velocity_bound = 50;
    pdesc.velocity_in_dir = true;
    pdesc.linear_scale = false;
  }
  //
  else if (key.find("enemy_death") != std::string::npos) {
    pdesc.size_curve = { { 32, 32 }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(0.8 * 1000);
    // pdesc.start_colour = hex_to_srgb("#a64a2e"); // dark red
    pdesc.start_colour = hex_to_srgb("#0096ff"); // death col
    pdesc.random_velocity_bound = 20;
  }
  //
  else if (key.find("death_exploder") != std::string::npos) {
    pdesc.size_curve = { { req.radius_pixels_upper, req.radius_pixels_upper }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(0.75 * 1000);
    pdesc.random_radius_bound_lower = -req.radius_pixels_upper;
    pdesc.random_radius_bound_upper = req.radius_pixels_upper;
    pdesc.random_velocity_bound = 50;
    pdesc.velocity_in_dir = true;
    pdesc.make_darker_based_on_distance_from_center = true;
    pdesc.start_colour = hex_to_srgb("#ffffff"); // death col
    pdesc.linear_scale = false;
  }
  //
  else if (key.find("death_turret_explode") != std::string::npos) {
    pdesc.size_curve = { { 3.0f * req.radius_pixels_upper, 3.0f * req.radius_pixels_upper }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(0.8f * 1000);
    pdesc.start_colour = hex_to_srgb("#b1c9c3"); // turret death col
    pdesc.linear_scale = false;
  }
  //
  else if (key.find("fire_particles") != std::string::npos) {
    pdesc.size_curve = { { 0, 0 }, { 4, 4 }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(1.0 * 1000);
    pdesc.start_colour = engine::SRGBColour{ 233, 159, 16, 255 }; // orangeish
    pdesc.random_radius_bound_upper = 16;
    pdesc.random_velocity_bound = 5;
  }
  //
  else if (key.find("ice_particles") != std::string::npos) {
    pdesc.size_curve = { { 0, 0 }, { 4, 4 }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(1.0 * 1000);
    pdesc.start_colour = hex_to_srgb("#90D1CA"); // blueish for ice
    pdesc.random_radius_bound_upper = 16;
    pdesc.random_velocity_bound = 5;
  }
  //
  else if (key.find("vfx_boop") != std::string::npos) {
    pdesc.size_curve = { { 8, 8 }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(0.8 * 1000);
    pdesc.start_colour = engine::SRGBColour{ 159, 233, 16, 255 }; // greenish
    pdesc.random_radius_bound_lower = 8;
    pdesc.random_radius_bound_upper = 8;
    pdesc.random_velocity_bound = 100;
    pdesc.velocity_in_dir = true;
  }
  //
  else if (key.find("vfx_ice_boop") != std::string::npos) {
    pdesc.size_curve = { { 8, 8 }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(0.8 * 1000);
    pdesc.start_colour = hex_to_srgb("#90D1CA"); // blueish for ice
    pdesc.random_radius_bound_lower = 8;
    pdesc.random_radius_bound_upper = 8;
    pdesc.random_velocity_bound = 50;
    pdesc.velocity_in_dir = true;
  }
  //
  else if (key.find("vfx_levelup_outer") != std::string::npos) {
    pdesc.size_curve = { { 0, 0 }, { 0, 0 }, { 6, 6 }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(1.75 * 1000);
    pdesc.start_colour = hex_to_srgb("#cfc041"); // gold_yellow
    pdesc.random_radius_bound_lower = 0;
    pdesc.random_radius_bound_upper = 0;
    pdesc.random_velocity_bound = 60;
    pdesc.velocity_in_dir = true;
    if (req.colour.has_value())
      pdesc.start_colour = req.colour.value();
    pdesc.linear_scale = false;
  }
  //
  else if (key.find("vfx_levelup_inner") != std::string::npos) {
    pdesc.size_curve = { { 0, 0 }, { 6, 6 }, { 0, 0 } };
    pdesc.time_to_live_ms = static_cast<int>(2.0 * 1000);
    pdesc.start_colour = hex_to_srgb("#cfc041"); // gold_yellow
    pdesc.random_radius_bound_lower = 32;
    pdesc.random_radius_bound_upper = 32;
    pdesc.random_velocity_bound = 60;
    pdesc.velocity_in_dir = true;
    pdesc.velocity_away = false;
    pdesc.linear_scale = false;
  }

  // which particle to spawn?
  ParticleEmitterComponent emitter;
  emitter.particle_to_emit = pdesc;
  emitter.expires = true;
  emitter.spawn_all_particles_at_once = true;
  if (key.find("default_trail") != std::string::npos) {
    emitter.expires = false;
    emitter.spawn_all_particles_at_once = false;
  } else if (key.find("default_explode") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 1;
  } else if (key.find("death_sea_mine") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 50;
  } else if (key.find("enemy_death") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 4;
  } else if (key.find("death_exploder") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 10;
  } else if (key.find("death_turret_explode") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 10;
  } else if (key.find("fire_particles") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 5;
  } else if (key.find("ice_particles") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 5;
  } else if (key.find("vfx_boop") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 30;
  } else if (key.find("vfx_ice_boop") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 10;
  } else if (key.find("vfx_levelup_outer") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 20;
  } else if (key.find("vfx_levelup_inner") != std::string::npos) {
    emitter.particles_to_spawn_before_emitter_expires = 10;
  }
  r.emplace<ParticleEmitterComponent>(e, emitter);

  // emit: particles
  CooldownComponent cooldown;
  cooldown.time_max = 0.1f;

  if (key.find("default_trail") != std::string::npos)
    cooldown.time_max = 1 / 30.0f; // 30 particle-a-second innit

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
  // set_sprite(r, e, "CIRCLE");

  r.emplace<EntityTimedLifecycle>(e, desc.time_to_live_ms);
  r.emplace<VelocityComponent>(e, VelocityComponent{ desc.velocity.x, desc.velocity.y });

  if (desc.fade)
    r.emplace<SetAlphaBasedOnLifecycleComponent>(e);

  // WARNING: random rotation here
  static engine::RandomState rnd(0);
  auto& t = r.get<TransformComponent>(e);
  t.rotation_radians.z = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI); // rnd rotation

  // make it shrink
  const ScaleOverTimeComponent sotc{
    .seconds_until_complete = desc.time_to_live_ms / 1000.0f,
    .size_curve = desc.size_curve,
    .linear = desc.linear_scale,
  };
  r.emplace<ScaleOverTimeComponent>(e, sotc);

  set_colour(r, e, desc.start_colour);
  set_position(r, e, desc.position);
  set_z_index(r, e, ZLayer::VFX);
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