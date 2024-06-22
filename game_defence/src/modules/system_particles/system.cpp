#include "system.hpp"

#include "components.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "modules/system_particles/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_particle_system(entt::registry& r, const float dt)
{
  // TEMP: should not be here. set position to parents position
  for (const auto& [e, request, has_parent] :
       r.view<const SetPositionToParentsPosition, const HasParentComponent>().each()) {
    const glm::ivec2 base_position = get_position(r, has_parent.parent);
    const glm::ivec2 pos = base_position + request.offset;
    set_position(r, e, pos);
  }

  // spawn the particles
  const auto& view = r.view<ParticleEmitterComponent, AttackCooldownComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, emitter, cooldown] : view.each()) {
    if (!cooldown.on_cooldown) {

      if (emitter.spawn_all_particles_at_once) {
        for (int i = 0; i < emitter.particles_to_spawn_before_emitter_expires; i++) {
          //

          // per-instance? seems bad
          ParticleDescription particle_description = emitter.particle_to_emit;

          // instead of spawning at emitter position, spawn at parent position
          if (auto* parent = r.try_get<HasParentComponent>(e))
            particle_description.position = get_position(r, parent->parent);

          if (emitter.random_velocity) {
            static engine::RandomState rnd;
            const float rnd_x = engine::rand_det_s(rnd.rng, -10, 10);
            // make particles go down? simulate gravity?
            const float rnd_y = engine::rand_det_s(rnd.rng, -10, 10);
            particle_description.velocity = glm::ivec2{ rnd_x, rnd_y };
          }

          create_particle(r, particle_description);

          // limit number of particles spawned
          if (emitter.expires) {
            emitter.particles_to_spawn_before_emitter_expires -= 1;
            if (emitter.particles_to_spawn_before_emitter_expires < 0) {
              r.destroy(e); // emitter expired!
              continue;
            }
          }

        } // end for-loop
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

    const float amount = glm::lerp(a, b, t);
    transform.scale = { static_cast<int>(amount), static_cast<int>(amount), 1 };

    scale.timer += dt;

    if (scale.timer >= scale.seconds_until_complete) {
      // particle done?
    }
  }
};

// todo: lerp particle colours?
//

} // namespace game2d
