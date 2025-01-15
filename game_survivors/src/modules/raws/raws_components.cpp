#include "modules/raws/raws_components.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_exploder/actor_exploder_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_enemy_projectile/enemy_projectile_helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"

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

engine::SRGBColour
colour_tag_to_colour(const Raws& raws, const std::string& col)
{
  for (const auto& c : raws.colours)
    if (c.tag == col)
      return hex_to_srgb(c.hex);
  return { 1.0f, 1.0f, 1.0f, 1.0f };
};

const auto item_body_type = b2_kinematicBody;
const auto mob_body_type = b2_kinematicBody;
const auto env_body_type = b2_kinematicBody;

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
  const auto& t = r.get<const Item>(e);
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
  if (t.physics_desc.has_value()) {
    PhysicsDescription pdesc;
    pdesc.type = b2_dynamicBody;
    pdesc.position = pos;
    pdesc.size = { size.x, size.y };
    pdesc.is_sensor = t.physics_desc->is_sensor;

    if (t.physics_desc->is_bullet.has_value())
      pdesc.is_bullet = t.physics_desc->is_bullet.value();

    if (t.physics_desc->is_static.has_value()) {
      const bool is_static = t.physics_desc->is_static.value();
      if (is_static)
        pdesc.type = b2_staticBody;
    }

    create_physics_actor(r, e, pdesc);
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
  const auto& templ = rs.items[idx];

  const auto e = r.create();
  r.emplace<TagComponent>(e, templ.name);
  r.emplace<WaitForInitComponent>(e);
  r.emplace<Item>(e, templ);
  r.emplace<OnDeathCallbacks>(e);

  bool big_explode = false;

  if (templ.traits.has_value()) {
    for (const auto& trait : templ.traits.value()) {
      if (trait.key == "direct") {
        ApplyForceToDynamicTarget tgt_c;
        tgt_c.orbit = false;
        tgt_c.reduce_thrusters = false;
        tgt_c.speed = 100.0f;
        r.emplace<ApplyForceToDynamicTarget>(e, tgt_c);
        //
      }
      if (trait.key == "projectile") {
        add_projectile_enemy_components(r, e);
      }
      if (trait.key == "explode") {
        add_explode_on_death_callback(r, e);
        big_explode = true;
      }
    }
  }

  // Spawn particles on death
  if (!big_explode) {
    auto& callbacks_c = r.get<OnDeathCallbacks>(e);
    const auto spawn_particles_callback = [](entt::registry& r, entt::entity e) {
      RequestToSpawnParticles request;
      request.key = "default_explode";
      request.position = get_position(r, e);
      create_empty<RequestToSpawnParticles>(r, request);
    };
    callbacks_c.callbacks.push_back(spawn_particles_callback);
  }

  if (templ.stats.has_value())
    r.emplace<HealthComponent>(e, templ.stats->hp, templ.stats->max_hp);
  r.emplace<DefenceComponent>(e, 0); // should be determined by equipment
  r.emplace<InputComponent>(e);

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
  if (key.find("death_exploder") != std::string::npos) {
    pdesc.start_size = explosion_radius * 2;
    pdesc.end_size = explosion_radius * 1;
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