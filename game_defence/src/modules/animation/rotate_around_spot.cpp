#include "modules/animation/rotate_around_spot.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_rotate_around_entity_system(entt::registry& r, const float& dt)
{
  const auto& view = r.view<RotateAroundEntity, HasTargetPositionComponent>();
  for (const auto& [e, info, target_pos] : view.each()) {

    // += is clockwise
    // -= is anticlockwise
    info.theta += dt * info.rotate_speed;

    const float& distance = info.distance;
    const glm::vec2& spot = get_position(r, info.e);
    const glm::vec2 dir = engine::angle_radians_to_direction(info.theta);
    const glm::vec2 target = spot + (dir * glm::vec2{ distance, distance });

    // set position seems incorrect,
    // but target_pos on a moving object was causing jittering
    set_position(r, e, target);

    // target_pos.position = target;
  }
};

} // namespace game2d