#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

static bool clamp_to_grid = false;

void
update_cursor_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  // check grid exists
  // const auto grid_e = get_first<GridComponent>(r);
  // if (grid_e == entt::null)
  //   return;
  // const auto& grid = r.get<GridComponent>(grid_e);

  // warning: doesnt work with controller currently
  const bool click = get_mouse_lmb_press();
  const bool held = get_mouse_lmb_held();
  const bool release = get_mouse_lmb_release();

  static glm::ivec2 click_location = { 0, 0 };
  if (click)
    click_location = mouse_pos;

  const auto& view = r.view<TransformComponent, AABB, CursorComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, transform, aabb, cursor] : view.each()) {

    // Is the cursor hovering any enemies?
    const auto& enemies_view = r.view<const HoveredComponent, const EnemyComponent>(entt::exclude<WaitForInitComponent>);
    if (enemies_view.size_hint() > 0 && !held)
      set_sprite(r, entity, "CURSOR_1");
    else
      set_sprite(r, entity, "EMPTY");

    // defaults
    glm::ivec2 size{ 0, 0 };
    glm::ivec2 pos = mouse_pos;

    // expand cursor when cursor is held
    if (held) {
      pos = (click_location + mouse_pos) / 2;
      size = glm::abs(mouse_pos - click_location);
    }

    set_position(r, entity, pos);
    set_size(r, entity, size);
  }
}

} // namespace game2d