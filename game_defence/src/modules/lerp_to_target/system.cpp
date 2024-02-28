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
  // set velocity to target

  const auto& view =
    r.view<VelocityComponent, const AABB, const HasTargetPositionComponent, const SetVelocityToTargetComponent>(
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

  //
  // This moves TransformComponent
  // That do not have AABB components.
  //
  const auto& t_view = r.view<VelocityComponent, TransformComponent>(entt::exclude<WaitForInitComponent, AABB>);
  for (const auto& [e, vel, t] : t_view.each()) {

    vel.remainder_x += vel.x * dt;
    vel.remainder_y += vel.y * dt;

    const int x = static_cast<int>(vel.remainder_x);
    const int y = static_cast<int>(vel.remainder_y);

    vel.remainder_x -= x;
    vel.remainder_y -= y;

    t.position.x += x;
    t.position.y += y;
  }
};

} // namespace game2d