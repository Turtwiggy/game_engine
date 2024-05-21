#include "helpers.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

entt::entity
create_particle(entt::registry& r, const ParticleDescription& desc)
{
  static engine::RandomState rnd;

  const auto e = create_gameplay(r, EntityType::particle);

  // time-alive
  r.get<EntityTimedLifecycle>(e).milliseconds_alive_max = desc.time_to_live_ms;

  // spawn-point
  auto& t = r.get<TransformComponent>(e);
  t.position = { desc.position.x, desc.position.y, 0.0f };
  t.rotation_radians.z = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI); // rnd rotation

  // velocity
  auto& vel = r.get<VelocityComponent>(e);
  vel.x = desc.velocity.x;
  vel.y = desc.velocity.y;

  // make it shrink
  ScaleOverTimeComponent sotc;
  sotc.seconds_until_complete = desc.time_to_live_ms;
  sotc.start_size = desc.start_size;
  sotc.end_size = desc.end_size;
  r.emplace<ScaleOverTimeComponent>(e, sotc);

  // update particle sprite to the correct sprite
  auto sc = create_sprite(r, desc.sprite, EntityType::particle);
  r.emplace_or_replace<SpriteComponent>(e, sc);

  return e;
};

} // namespace game2d