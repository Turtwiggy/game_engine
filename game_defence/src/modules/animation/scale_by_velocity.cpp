#include "modules/animation/scale_by_velocity.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "helpers.hpp"
#include "maths/maths.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/physics/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

#include <cmath>

namespace game2d {

void
update_scale_by_velocity_system(entt::registry& r)
{
  // note: ignore physics transform component
  // as this is purely a visual effect that
  // does not impact physics sizes

  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r);
  const auto cursor = get_first<PlayerCursor>(r);
  auto& cursor_transform = r.get<TransformComponent>(cursor);
  cursor_transform.position = { mouse_position.x, mouse_position.y, 0 };
  cursor_transform.rotation_radians.z = glm::radians(45.0f);
  cursor_transform.scale.x = 15;
  cursor_transform.scale.y = 15;
  const RotatedSquare cursor_square = transform_to_rotated_square(cursor_transform);

  bool any_collision = false;

  const auto& view = r.view<TransformComponent, const VelocityComponent, ScaleTransformByVelocity>();
  for (const auto& [entity, transform, velocity, scale] : view.each()) {

    // set bullet angle
    glm::vec2 dir = { velocity.x, velocity.y };
    if (dir.x != 0.0f || dir.y != 0.0f)
      dir = glm::normalize(dir);
    transform.rotation_radians.z = engine::dir_to_angle_radians(dir);

    // HMM
    transform.scale.x = 25;
    transform.scale.y = 25;

    // after rotation, caculate rotated square points
    //
    const RotatedSquare square = transform_to_rotated_square(transform);

    // hack: change colour of bullet if mouse is hovering on it
    auto& col = r.get<SpriteColourComponent>(entity);
    // if (point_in_rectangle(square, mouse_position))
    //   col.colour->g = 1.0f;
    // else
    //   col.colour->g = 0.0f;

    // hack: check if mouse object is colliding with this bullet
    const auto collision = rotated_squares_collide(square, cursor_square);
    if (collision)
      col.colour->g = 1.0f;
    else
      col.colour->g = 0.0f;

    r.get<TransformComponent>(scale.debug_a).position = { square.tl.x, square.tl.y, 0 };
    r.get<TransformComponent>(scale.debug_a).scale = { 3, 3, 1 };
    r.get<TransformComponent>(scale.debug_b).position = { square.tr.x, square.tr.y, 0 };
    r.get<TransformComponent>(scale.debug_b).scale = { 3, 3, 1 };
    r.get<TransformComponent>(scale.debug_c).position = { square.bl.x, square.bl.y, 0 };
    r.get<TransformComponent>(scale.debug_c).scale = { 3, 3, 1 };
    r.get<TransformComponent>(scale.debug_d).position = { square.br.x, square.br.y, 0 };
    r.get<TransformComponent>(scale.debug_d).scale = { 3, 3, 1 };
  }

  // if (any_collision)
  //   cursor_col.colour->g = 1.0f;
  // else
  //   cursor_col.colour->g = 0.0f;
}

} // namespace game2d