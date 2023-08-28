#include "system.hpp"

#include "components.hpp"
#include "physics//components.hpp"
#include "renderer/components.hpp"

namespace game2d {

void
update_cursor_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& view = r.view<TransformComponent, AABB, const CursorComponent>();
  for (const auto& [entity, transform, aabb, cursor] : view.each()) {
    transform.position.x = mouse_pos.x;
    transform.position.y = mouse_pos.y;
    transform.scale.x = 16;
    transform.scale.y = 16;
    aabb.center.x = mouse_pos.x;
    aabb.center.y = mouse_pos.y;
    aabb.size = { 16, 16 };
  }
}

} // namespace game2d