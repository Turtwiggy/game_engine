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
  const auto& view = r.view<VelocityComponent, const AABB, const LerpToTargetComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, vel, aabb, lerp] : view.each()) {
    glm::vec2 target{ 0, 0 };

    //   // and multiply it by the offset of the weapon
    //   const glm::ivec2 offset = { input.rx * player.weapon_offset, input.ry * player.weapon_offset };

    //   // Set Lerp Target
    //   target.position = player_aabb.center + offset;

    // lerp to aabb
    if (const auto* parent = r.try_get<HasParentComponent>(e)) {
      const auto* parent_aabb = r.try_get<AABB>(parent->parent);
      if (parent_aabb)
        target = parent_aabb->center;
    }
    // lerp to transform
    // if(//)
    // position to lerp to not set
    else
      continue;

    //
    const glm::vec2 a = { aabb.center.x, aabb.center.y };
    const glm::vec2 b = target;

    glm::vec2 dir = b - a;
    if (dir.x != 0.0f || dir.y != 0.0f)
      dir = glm::normalize(dir);

    vel.x = dir.x * lerp.speed;
    vel.y = dir.y * lerp.speed;

    // std::cout << "vel x " << vel.x << " y: " << vel.y << std::endl;
  }
};

} // namespace game2d