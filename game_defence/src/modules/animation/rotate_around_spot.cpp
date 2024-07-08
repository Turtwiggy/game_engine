#include "modules/animation/rotate_around_spot.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_rotate_around_spot_system(entt::registry& r, const float& dt)
{
  const auto& view = r.view<RotateAroundSpot, TransformComponent>();
  for (const auto& [e, rotate_info, transform] : view.each()) {

    rotate_info.theta += dt * rotate_info.rotate_speed;

    // dir to offset the rotation object
    const glm::vec2 dir = engine::angle_radians_to_direction(rotate_info.theta);

    const glm::vec2 offset = dir * rotate_info.distance;
    set_position(r, e, { rotate_info.spot.x + offset.x, rotate_info.spot.x + offset.y });

    transform.rotation_radians.z = rotate_info.theta;
  }
};

} // namespace game2d