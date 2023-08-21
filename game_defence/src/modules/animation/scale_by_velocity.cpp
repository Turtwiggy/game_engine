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

  static float rotation = 0.0f;
  rotation += dt * 10;

  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_B))
    std::cout << "breakpoint!\n";
  cursor_transform.rotation_radians.z = glm::radians(rotation);
  cursor_transform.scale = { 16, 32, 1 };
  cursor_transform.position = { mouse_position.x, mouse_position.y, 0 };
  const RotatedSquare cursor_square = transform_to_rotated_square(cursor_transform);
  const AABB aabb_cursor = generate_aabb(cursor_square);
  {
    const auto& cursor_component = r.get<PlayerCursor>(cursor);
    const float w = aabb_cursor.x.r - aabb_cursor.x.l;
    const float h = aabb_cursor.y.t - aabb_cursor.y.b;
    r.get<TransformComponent>(cursor_component.debug_cursor).position = { aabb_cursor.center.x, aabb_cursor.center.y, 0 };
    r.get<TransformComponent>(cursor_component.debug_cursor).scale = { w, h, 1 };
  }

  static float bullet_rotation = 0.0f;
  bullet_rotation += 2 * dt * 10;

  const auto& view = r.view<TransformComponent, const VelocityComponent, ScaleTransformByVelocity>();
  for (const auto& [entity, transform, velocity, scale] : view.each()) {

    // set bullet angle
    glm::vec2 dir = { velocity.x, velocity.y };
    if (dir.x != 0.0f || dir.y != 0.0f)
      dir = glm::normalize(dir);
    // transform.rotation_radians.z = engine::dir_to_angle_radians(dir);
    transform.rotation_radians.z = glm::radians(bullet_rotation);

    // HMM
    transform.scale.x = 10;
    transform.scale.y = 40;

    // after rotation, caculate rotated square points
    //
    const RotatedSquare square = transform_to_rotated_square(transform);

    const AABB aabb_square = generate_aabb(square);
    {
      const int w = glm::abs(aabb_square.x.r - aabb_square.x.l);
      const int h = glm::abs(aabb_square.y.t - aabb_square.y.b);
      r.get<TransformComponent>(scale.debug_aabb).position = { aabb_square.center.x, aabb_square.center.y, 0 };
      r.get<TransformComponent>(scale.debug_aabb).scale = { w, h, 1 };
    }

    auto& col = r.get<SpriteColourComponent>(entity);

    bool any_collision = false;

    // Broadphase: Check AABB
    if (collide(aabb_square, aabb_cursor)) {
      col.colour->r = 0.0f;
      col.colour->g = 0.5f;
      col.colour->b = 0.0f;
      any_collision = true;
    }

    // Narrowphase: GJK
    if (gjk_squares_collide(square, cursor_square)) {
      col.colour->r = 0.0f;
      col.colour->g = 1.0f;
      col.colour->b = 0.0f;
      any_collision = true;
    }

    if (!any_collision) {
      col.colour->r = 1.0f;
      col.colour->g = 1.0f;
      col.colour->b = 1.0f;
    }
  }
}

} // namespace game2d