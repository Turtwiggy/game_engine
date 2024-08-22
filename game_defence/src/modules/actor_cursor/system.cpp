#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "modules/actors/helpers.hpp"

namespace game2d {

void
update_cursor_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  // warning: doesnt work with controller currently
  const bool click = get_mouse_lmb_press();
  const bool held = get_mouse_lmb_held();
  const bool release = get_mouse_lmb_release();

  const auto cursor_e = get_first<CursorComponent>(r);
  if (cursor_e == entt::null)
    return;

  auto& cursor_c = get_first_component<CursorComponent>(r);
  if (click)
    cursor_c.click_location = mouse_pos;

  const auto& view = r.view<CursorComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, cursor] : view.each()) {

    // defaults
    glm::ivec2 size{ 0, 0 };
    glm::ivec2 pos = mouse_pos;

    // expand cursor when cursor is held
    if (cursor_c.click_location.has_value() && held) {
      pos = (cursor_c.click_location.value() + mouse_pos) / 2;
      size = glm::abs(mouse_pos - cursor_c.click_location.value());
    }

    set_position(r, entity, pos);
    set_size(r, entity, size);
  }
}

} // namespace game2d