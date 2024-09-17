#include "components.hpp"

#include "actors/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/colour/components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"

#include <fstream>
#include <sstream>

namespace game2d {

Raws
load_raws(std::string path)
{
  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data = buffer.str();

  nlohmann::json root = nlohmann::json::parse(data);
  const auto raws = root.get<Raws>();
  return raws;
};

entt::entity
create_transform(entt::registry& r, std::string name)
{
  const auto e = r.create();
  r.emplace<TagComponent>(e, name);
  r.emplace<WaitForInitComponent>(e);
  r.emplace<TransformComponent>(e);
  return e;
};

entt::entity
spawn_item(entt::registry& r, const std::string& key, const glm::vec2& pos)
{
  const auto& rs = get_first_component<Raws>(r);

  const auto it = std::find_if(rs.items.begin(), rs.items.end(), [&key](const Item& item) { return item.name == key; });
  if (it == rs.items.end()) {
    fmt::println("unable to find item in raw file: {}", key);
    return entt::null;
  };

  const auto idx = static_cast<int>(it - rs.items.begin());
  const Item& item_template = rs.items[idx];

  const auto e = r.create();
  r.emplace<TagComponent>(e, item_template.name);
  r.emplace<WaitForInitComponent>(e);

  // create_transform()
  {
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, item_template.renderable.sprite);
    set_colour(r, e, item_template.renderable.colour);

    TransformComponent tf;
    tf.position = { pos.x, pos.y, 0.0f };
    tf.scale = { 32, 32, 0.0f };
    r.emplace<TransformComponent>(e, tf);
    set_position(r, e, { tf.position.x, tf.position.y });
    set_size(r, e, { 32, 32 });
  }

  if (item_template.use.has_value())
    r.emplace<Use>(e);

  return e;
};

entt::entity
spawn_mob(entt::registry& r, const std::string& key, const glm::vec2& pos)
{
  const auto& rs = get_first_component<Raws>(r);

  const auto it = std::find_if(rs.mobs.begin(), rs.mobs.end(), [&key](const Mob& item) { return item.name == key; });
  if (it == rs.mobs.end()) {
    fmt::println("unable to find item in raw file: {}", key);
    return entt::null;
  };

  const auto idx = static_cast<int>(it - rs.mobs.begin());
  const Mob& item_template = rs.mobs[idx];

  const auto e = r.create();
  r.emplace<TagComponent>(e, item_template.name);
  r.emplace<WaitForInitComponent>(e);

  // create_transform()
  {
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, item_template.renderable.sprite);
    set_colour(r, e, item_template.renderable.colour);
    r.emplace<DefaultColour>(e, item_template.renderable.colour);

    TransformComponent tf;
    tf.position = { pos.x, pos.y, 0.0f };
    tf.scale = { 16, 16, 0.0f };
    r.emplace<TransformComponent>(e, tf);
  }

  // create_physics()
  {
    PhysicsDescription pdesc;
    pdesc.type = b2_dynamicBody;
    pdesc.position = pos;
    pdesc.size = { 16, 16 };
    pdesc.is_sensor = false;
    create_physics_actor(r, e, pdesc);
  }

  // if (item_template.stats.){
  // }

  return e;
};

entt::entity
spawn_particle_emitter(entt::registry& r, const std::string& key, const glm::vec2& pos, entt::entity& parent)
{
  const auto e = create_transform(r, "particle_emitter");

  r.emplace<SetPositionAtDynamicTarget>(e);
  r.emplace<DynamicTargetComponent>(e, parent);

  Particle pdesc;
  pdesc.start_size = 6;
  pdesc.end_size = 2;
  pdesc.time_to_live_ms = 1 * 1000;

  // which particle to spawn?
  ParticleEmitterComponent pedesc;
  pedesc.particle_to_emit = pdesc;
  r.emplace<ParticleEmitterComponent>(e, pedesc);

  // emit: particles
  CooldownComponent cooldown;
  cooldown.time = 0.1f;
  cooldown.time_left = cooldown.time;
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