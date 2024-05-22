#include "system.hpp"

#include "components.hpp"
#include "lifecycle/components.hpp"
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
  const auto& view = r.view<const ParticleEmitterComponent, const HasParentComponent, AttackCooldownComponent>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [e, emitter, parent, cooldown] : view.each()) {
    if (!cooldown.on_cooldown) {

      const auto parent_position = get_position(r, parent.parent);
      // const auto parent_velocity = r.get<VelocityComponent>(parent.parent);

      const ParticleDescription base_description = emitter.particle_to_emit;
      // per-instance? seems bad
      ParticleDescription particle_description = base_description;
      particle_description.position = parent_position;
      particle_description.velocity = { 0, 0 };
      particle_description.start_size = 6;
      particle_description.end_size = 0;
      particle_description.time_to_live_ms = 1 * 1000;
      create_particle(r, particle_description);

      reset_cooldown(cooldown);
    }
  }

  // Scale particles
  const auto& particle_view = r.view<TransformComponent, ScaleOverTimeComponent, const EntityTimedLifecycle>();
  for (const auto& [e, transform, scale, life] : particle_view.each()) {

    // if ((life.milliseconds_alive_max / 1000) < scale.seconds_until_complete) {
    //   std::cout << "warning: particle scaleoversize time is less than lifecycle time; will dissapear before completing"
    //             << std::endl;
    // }

    const float lerp_amount = scale.timer / scale.seconds_until_complete;
    const float amount = glm::lerp(scale.start_size, scale.end_size, lerp_amount);
    transform.scale = { static_cast<int>(amount), static_cast<int>(amount), 1 };
    scale.timer += dt;
  }
};

// todo: lerp particle colours?
//

} // namespace game2d
