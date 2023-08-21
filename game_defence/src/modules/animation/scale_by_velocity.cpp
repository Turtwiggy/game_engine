#include "modules/animation/scale_by_velocity.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "helpers.hpp"
#include "maths/maths.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

#include <cmath>

namespace game2d {

void
update_scale_by_velocity_system(entt::registry& r, float dt)
{
  // place a cursor and visualise its aabb
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r);
  const auto cursor = get_first<PlayerCursor>(r);
  auto& cursor_transform = r.get<TransformComponent>(cursor);
  cursor_transform.scale = { 2, 2, 1 };
  cursor_transform.position = { mouse_position.x, mouse_position.y, 0 };
  const auto cursor_square = transform_to_rotated_square(cursor_transform);
  const auto cursor_aabb = generate_aabb(cursor_square);

  const auto& view = r.view<TransformComponent, const VelocityComponent, const ScaleTransformByVelocity>();
  for (const auto& [entity, transform, velocity, scale] : view.each()) {

    // set bullet angle
    glm::vec2 dir = { velocity.x, velocity.y };
    if (dir.x != 0.0f || dir.y != 0.0f)
      dir = glm::normalize(dir);
    transform.rotation_radians.z = engine::dir_to_angle_radians(dir);

    // after rotation, caculate rotated square points
    //
    const RotatedSquare square = transform_to_rotated_square(transform);

    bool any_collision = false;
    auto& col = r.get<SpriteColourComponent>(entity);

    // Broadphase: Check AABB
    const AABB bullet_aabb = generate_aabb(square);
    if (collide(bullet_aabb, cursor_aabb)) {

      any_collision = true;
      col.colour->r = 0.0f;
      col.colour->g = 0.5f;
      col.colour->b = 0.0f;

      // Narrowphase: GJK
      if (gjk_squares_collide(square, cursor_square))
        col.colour->g = 1.0f;
    }

    // reset colour
    if (!any_collision) {
      col.colour->r = 1.0f;
      col.colour->g = 1.0f;
      col.colour->b = 1.0f;
    }
  }
}

} // namespace game2d