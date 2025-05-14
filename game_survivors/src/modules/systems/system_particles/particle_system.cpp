#include "particle_system.hpp"

#include "components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_cooldown/helpers.hpp"
#include "modules/systems/system_move_to_target_via_lerp/components.hpp"

namespace game2d {

void
update_particle_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto spawn_particle_helper = [&r](const ParticleEmitterComponent& emitter, const entt::entity e) {
    // per-instance? seems bad
    auto particle_description = emitter.particle_to_emit;

    // spawn at e
    particle_description.position = get_position(r, e);

    if (auto* target_c = r.try_get<DynamicTargetComponent>(e)) {
      if (!r.valid(target_c->target)) {
        r.remove<DynamicTargetComponent>(e);
        return;
      }
      particle_description.position = get_position(r, target_c->target);
    }

    if (emitter.random_velocity) {
      static engine::RandomState rnd;
      const int rnd_x = engine::rand_det_s(rnd.rng, -50, 50);
      const int rnd_y = engine::rand_det_s(rnd.rng, -50, 50);
      particle_description.velocity = glm::ivec2{ rnd_x, rnd_y };
    }

    const auto p = Particle{
      .time_to_live_ms = particle_description.time_to_live_ms,
      .start_size = particle_description.start_size,
      .end_size = particle_description.end_size,
      .position = particle_description.position,
    };
    const entt::entity particle_e = spawn_particle(r, "default_particle", p);
    set_z_index(r, particle_e, ZLayer::BEHIND_PLAYER);
  };

  // spawn the particles
  const auto& view = r.view<ParticleEmitterComponent, CooldownComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [particle_emitter_e, emitter, cooldown] : view.each()) {

    if (cooldown.time <= 0.0f) {

      if (emitter.spawn_all_particles_at_once) {
        for (int i = 0; i < emitter.particles_to_spawn_before_emitter_expires; i++) {
          spawn_particle_helper(emitter, particle_emitter_e);
          emitter.particles_to_spawn_before_emitter_expires--;
        }
      } else {
        spawn_particle_helper(emitter, particle_emitter_e);
        emitter.particles_to_spawn_before_emitter_expires--;
      }

      // limit number of particles spawned
      if (emitter.expires && emitter.particles_to_spawn_before_emitter_expires < 0) {
        r.destroy(particle_emitter_e); // emitter expired!
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

    const float amount_x = engine::lerp(a.x, b.x, t);
    const float amount_y = engine::lerp(a.y, b.y, t);
    transform.scale = { amount_x, amount_y, 1 };

    scale.timer += dt;

    if (scale.timer >= scale.seconds_until_complete)
      scale.timer = scale.seconds_until_complete; // done?
  }
};

} // namespace game2d
