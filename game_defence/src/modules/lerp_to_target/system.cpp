#include "system.hpp"

#include "components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
// #include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_set_velocity_to_target_system(entt::registry& r, const float& dt)
{
  // lerp to position
  const auto& view = r.view<VelocityComponent, const AABB, const HasTargetPositionComponent, const LerpToTargetComponent>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [e, vel, aabb, target, lerp] : view.each()) {

    const glm::ivec2 a = { aabb.center.x, aabb.center.y };
    const glm::ivec2 b = target.position;

    glm::vec2 dir = b - a;
    if (dir.x != 0 || dir.y != 0.0f)
      dir = glm::normalize(dir);

    vel.x = dir.x * lerp.speed;
    vel.y = dir.y * lerp.speed;
    // std::cout << "vel x " << vel.x << " y: " << vel.y << std::endl;
  }
};

} // namespace game2d