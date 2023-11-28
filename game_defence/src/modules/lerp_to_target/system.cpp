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
  const auto& view = r.view<VelocityComponent, const HasTargetPositionComponent, const LerpToTargetComponent>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, transform, target_t, lerp] : view.each()) {
    // const glm::vec2 start = { transform.position.x, transform.position.y };
    // const glm::vec2 end = target_t.position;
    // const float t = 1.0f - glm::pow(0.5f, dt * lerp.speed);
    // const glm::vec2 new_pos = glm::lerp(start, end, t);
    // transform.position = { new_pos.x, new_pos.y, 0.0f };
    //
  }

  //
  // get bow to follow player
  //
  // const auto& v = r.view<const BowComponent, const HasParentComponent, HasTargetPositionComponent, TransformComponent>(
  //   entt::exclude<WaitForInitComponent>);
  // for (const auto& [e, bow, parent, target, t] : v.each()) {
  //   const auto& p = parent.parent;
  //   if (p == entt::null)
  //     continue;
  //   const auto& [player, player_aabb, input] = r.get<const PlayerComponent, const AABB, InputComponent>(p);

  //   // get the right analogue input of the player
  //   // and multiply it by the offset of the weapon
  //   const glm::ivec2 offset = { input.rx * player.weapon_offset, input.ry * player.weapon_offset };

  //   // Set Lerp Target
  //   target.position = player_aabb.center + offset;

  //   // Set Angle of bow
  //   const float bow_angle = engine::dir_to_angle_radians(offset) - engine::PI;
  //   t.rotation_radians.z = bow_angle;

  //   const auto d = glm::ivec2{ target.position.x, target.position.y } - player_aabb.center;
  //   const auto d2 = d.x * d.x + d.y * d.y;
  //   if (d2 < 10)
  //     t.rotation_radians.z = 45.0f * engine::Deg2Rad;
  // }
};

} // namespace game2d