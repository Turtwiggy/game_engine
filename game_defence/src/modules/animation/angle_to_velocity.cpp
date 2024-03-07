#include "angle_to_velocity.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_scale_by_velocity_system(entt::registry& r, float dt)
{
  // // place a cursor and visualise its aabb
  // const glm::ivec2 mouse_position = mouse_position_in_worldspace(r);
  // const auto cursor = get_first<PlayerCursor>(r);
  // auto& cursor_transform = r.get<TransformComponent>(cursor);
  // cursor_transform.scale = { 2, 2, 1 };
  // cursor_transform.position = { mouse_position.x, mouse_position.y, 0 };
  // const auto cursor_square = transform_to_rotated_square(cursor_transform);
  // const auto cursor_aabb = generate_aabb(cursor_square);

  const auto& view = r.view<TransformComponent, const VelocityComponent, const SetTransformAngleToVelocity>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, transform, velocity, scale] : view.each()) {
    // set angle
    const glm::vec2 raw_dir = { velocity.x, velocity.y };
    const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);
    transform.rotation_radians.z = engine::dir_to_angle_radians(nrm_dir);

    //   // Set Angle of bow
    //   const float bow_angle = engine::dir_to_angle_radians(offset) - engine::PI;
    //   t.rotation_radians.z = bow_angle;
    //   const auto d = glm::ivec2{ target.position.x, target.position.y } - player_aabb.center;
    //   const auto d2 = d.x * d.x + d.y * d.y;
    //   if (d2 < 10)
    //     t.rotation_radians.z = 45.0f * engine::Deg2Rad;
    // }
  }
}

} // namespace game2d