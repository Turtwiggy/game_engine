#include "breach_charge_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_breach_charge/breach_charge_helpers.hpp"
#include "modules/map/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

#include <fmt/core.h>

namespace game2d {

void
update_breach_charge_system(entt::registry& r, const glm::ivec2& mouse_pos, const float dt)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);

  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  if (!ri.viewport_hovered)
    return; // no ui

  const auto offset = glm::vec2{ map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
  const auto mouse_pos_on_grid = engine::grid::worldspace_to_clamped_world_space(mouse_pos, map_c.tilesize) + offset;

  // do the rest of the logic if not debugging this
  // if (debug_spawn_bombs(r, mouse_pos_on_grid))
  //   return;

  // only place bombs if bomb equipped
  const auto inv_bomb_e = bomb_equipped_in_inventory(r);
  if (inv_bomb_e == entt::null)
    return;

  // static here is wrong...
  static float lmb_held_time = 0.0f;
  static float lmb_time_to_place_bomb = 2.5f;

  // get lmb here is wrong...
  if (get_mouse_lmb_held())
    lmb_held_time += dt;

  if (get_mouse_lmb_release())
    lmb_held_time = 0.0f;

  const bool cursor_complete_so_place_bomb = lmb_held_time > lmb_time_to_place_bomb;
  if (cursor_complete_so_place_bomb)
    lmb_held_time = 0.0f;
  // ImGui::Text("Bomb Held time: %f", lmb_held_time);

  // Convert from [0, time_to_place_bomb] to [0, 2*PI]
  const float angle = engine::scale(lmb_held_time, 0.0f, lmb_time_to_place_bomb, 0.0f, 2.0f * engine::PI);
  DrawZeldaCursorWindow(r, mouse_pos, angle);

  if (cursor_complete_so_place_bomb) {
    fmt::println("spawning bomb!");

    // spawn the bomb!
    const auto world_bomb_e = spawn_item(r, "breach_charge");
    set_position(r, world_bomb_e, mouse_pos_on_grid);

    // remove bomb from inventory (a one use)
    const auto& item_e = r.get<UI_ItemComponent>(inv_bomb_e);
    r.get<InventorySlotComponent>(item_e.parent_slot).item_e = entt::null;
    r.destroy(inv_bomb_e); // destroy item from inventory
  }
};

} // namespace game2d