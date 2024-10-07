#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"

namespace game2d {

void
update_particle_system(entt::registry& r, const float dt)
{
  const auto spawn_particle_helper = [&r](const ParticleEmitterComponent& emitter, const entt::entity e) {
    // per-instance? seems bad
    auto particle_description = emitter.particle_to_emit;

    // instead of spawning at emitter position, spawn at parent position
    particle_description.position = get_position(r, e);
    if (auto* target_c = r.try_get<DynamicTargetComponent>(e))
      particle_description.position = get_position(r, target_c->target);

    if (emitter.random_velocity) {
      static engine::RandomState rnd;
      const int rnd_x = engine::rand_det_s(rnd.rng, -50, 50);
      const int rnd_y = engine::rand_det_s(rnd.rng, -50, 50);
      particle_description.velocity = glm::ivec2{ rnd_x, rnd_y };
    }

    Particle p;
    p.position = particle_description.position;
    p.start_size = particle_description.start_size;
    p.end_size = particle_description.end_size;
    p.time_to_live_ms = particle_description.time_to_live_ms;
    const entt::entity particle_e = spawn_particle(r, "default_particle", p);
    set_z_index(r, particle_e, ZLayer::BEHIND_PLAYER);
  };

  // spawn the particles
  const auto& view = r.view<ParticleEmitterComponent, CooldownComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, emitter, cooldown] : view.each()) {

    if (cooldown.time <= 0.0f) {

      if (emitter.spawn_all_particles_at_once) {
        for (int i = 0; i < emitter.particles_to_spawn_before_emitter_expires; i++) {
          spawn_particle_helper(emitter, e);
          emitter.particles_to_spawn_before_emitter_expires--;
        }
      } else {
        spawn_particle_helper(emitter, e);
        emitter.particles_to_spawn_before_emitter_expires--;
      }

      // limit number of particles spawned
      if (emitter.expires && emitter.particles_to_spawn_before_emitter_expires < 0) {
        r.destroy(e); // emitter expired!
        continue;
      }

      reset_cooldown(cooldown);
    }
  }

  // Scale particles
  const auto& particle_view = r.view<TransformComponent, ScaleOverTimeComponent, const EntityTimedLifecycle>();
  for (const auto& [e, transform, scale, life] : particle_view.each()) {

    const auto& a = scale.start_size;
    const auto& b = scale.end_size;

    float t = scale.timer / scale.seconds_until_complete;
    if (t >= 1.0f)
      t = 1.0f;

    const float amount = engine::lerp(a, b, t);
    transform.scale = { static_cast<int>(amount), static_cast<int>(amount), 1 };

    scale.timer += dt;

    if (scale.timer >= scale.seconds_until_complete) {
      // particle done?
    }
  }
};

} // namespace game2d
