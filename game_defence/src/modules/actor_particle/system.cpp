#include "system.hpp"

#include "components.hpp"
#include "modules/lifecycle/components.hpp"
#include "renderer/transform.hpp"

#include <glm/gtx/compatibility.hpp> // lerp

#include <iostream> // temp

namespace game2d {

void
update_particle_system(entt::registry& r, const float& dt)
{
  // Scale down particles
  //
  const auto& particle_view =
    r.view<const ParticleComponent, TransformComponent, ScaleOverTimeComponent, const EntityTimedLifecycle>();
  for (const auto& [e, particle, transform, scale, life] : particle_view.each()) {

    if ((life.milliseconds_alive_max / 1000) < scale.seconds_until_complete) {
      std::cout << "warning: particle scaleoversize time is less than lifecycle time; will dissapear before completing"
                << std::endl;
    }

    const float lerp_amount = scale.timer / scale.seconds_until_complete;
    const float amount = glm::lerp(scale.start_size, scale.end_size, lerp_amount);
    transform.scale.x = amount;
    transform.scale.y = amount;

    scale.timer += dt;
  }
}

} // namespace game2d