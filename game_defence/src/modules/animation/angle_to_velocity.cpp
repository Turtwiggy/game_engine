#include "angle_to_velocity.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "physics//components.hpp"
#include "renderer/components.hpp"

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

  const auto& view = r.view<TransformComponent, const VelocityComponent, const SetTransformAngleToVelocity>();
  for (const auto& [entity, transform, velocity, scale] : view.each()) {
    // set angle
    glm::vec2 dir = { velocity.x, velocity.y };
    if (dir.x != 0.0f || dir.y != 0.0f)
      dir = glm::normalize(dir);
    transform.rotation_radians.z = engine::dir_to_angle_radians(dir);
  }
}

} // namespace game2d