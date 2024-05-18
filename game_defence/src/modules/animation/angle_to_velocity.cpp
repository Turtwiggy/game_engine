#include "angle_to_velocity.hpp"

#include "components.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_scale_by_velocity_system(entt::registry& r, float dt)
{
  const auto& view = r.view<TransformComponent, const VelocityComponent, const SetTransformAngleToVelocity>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, transform, velocity, scale] : view.each()) {
    const glm::vec2 raw_dir = { velocity.x, velocity.y };
    const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);

    const float d2 = nrm_dir.x * nrm_dir.x + nrm_dir.y * nrm_dir.y;
    if (d2 > 0.0f)
      transform.rotation_radians.z = engine::dir_to_angle_radians(nrm_dir);
  }
}

} // namespace game2d