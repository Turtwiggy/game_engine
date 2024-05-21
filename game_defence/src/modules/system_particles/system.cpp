#include "system.hpp"

#include "components.hpp"
#include "lifecycle/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "modules/system_particles/helpers.hpp"
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

  // Scale particles
  const auto& particle_view = r.view<TransformComponent, ScaleOverTimeComponent, const EntityTimedLifecycle>();
  for (const auto& [e, transform, scale, life] : particle_view.each()) {
    const float lerp_amount = scale.timer / scale.seconds_until_complete;
    const float amount = glm::lerp(scale.start_size, scale.end_size, lerp_amount);
    transform.scale = { amount, amount, 1 };
    scale.timer += dt;
  }

  // spawn the particles
  const auto& view = r.view<const ParticleEmitterComponent, AttackCooldownComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, emitter, cooldown] : view.each()) {
    if (!cooldown.on_cooldown) {
      reset_cooldown(cooldown);

      create_particle(r, emitter.particle_to_emit);
    }
  }
};

} // namespace game2d