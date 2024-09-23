#include "system.hpp"

#include "actors/helpers.hpp"
#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/map/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_inventory/components.hpp"

#include "imgui.h"
#include "modules/ui_inventory/helpers.hpp"

namespace game2d {

void
toggle_lootbag_display(entt::registry& r)
{
  const auto& input = get_first_component<SINGLE_InputComponent>(r);

  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_R)) {
    // not showing => showing
    if (get_first<ShowLootbagRequest>(r) == entt::null)
      destroy_first_and_create<ShowLootbagRequest>(r);
    // showing => not showing
    else
      destroy_first<ShowLootbagRequest>(r);
  }
};

void
update_ui_lootbag_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = get_first_component<MapComponent>(r);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  toggle_lootbag_display(r);

  // no request: do not show loot menu
  if (get_first<ShowLootbagRequest>(r) == entt::null)
    return;
  auto& ui = get_first_component<SINGLE_UI_Lootbag>(r);

  const int inv_x = 6;
  const ImVec2 button_size = ImVec2(32, 32); // make the border 48 or 64
  const ImVec2 window_0_size{ button_size.x * 8, button_size.y * (inv_x + 1) };

  const float window_left_edge_padding = 20;
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const float pos_x = viewport_pos.x + ri.viewport_size_current.x - window_0_size.x;
  const float pos_y = viewport_pos.y + viewport_size_half.y;
  const auto pos = ImVec2(pos_x, pos_y);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  // flags |= ImGuiWindowFlags_NoInputs;

  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 0.5f));
  ImGui::SetNextWindowSizeConstraints(window_0_size, window_0_size);
  ImGui::PushStyleVar(ImGuiTableColumnFlags_WidthFixed, button_size.x);

  ImGui::Begin("UILootbag", NULL, flags);
  ui.hovered = ImGui::IsWindowHovered();

  ImGui::SeparatorText("Loot (Press R to toggle)");
  ImGui::Text("Right click to take loot");

  const auto content_size = ImGui::GetContentRegionAvail();

  for (const auto& [e, player_c] : r.view<PlayerComponent>().each()) {
    ImGui::SeparatorText("player...");

    const auto gp = get_grid_position(r, e);

    const auto idx = engine::grid::grid_position_to_index(gp, map_c.xmax);
    if (idx < 0 || idx > map_c.xmax * map_c.ymax)
      continue; // unlikely to be loot out of bounds?

    // remove the player from the grid cell index,
    // and assume that every other entity is an inventory.
    auto es = map_c.map[idx]; // take a copy
    auto it = std::remove(es.begin(), es.end(), e);
    if (it != es.end())
      es.erase(it, es.end());

    const std::string info_str = std::format("Standing on {} lootbags...", es.size());
    ImGui::Text("%s", info_str.c_str());

    // should be all inventories
    for (const auto lootbag_e : es) {

      const auto& inv_c = r.get<DefaultInventory>(lootbag_e);

      bool all_items_empty = true;

      for (const auto& inv_e : inv_c.inv) {
        const auto& slot_c = r.get<InventorySlotComponent>(inv_e);
        const auto item_e = slot_c.item_e;
        if (item_e != entt::null)
          all_items_empty = false;
      }

      if (all_items_empty) {
        ImGui::SeparatorText("Lootbag... (empty)");
        continue;
      }
      ImGui::SeparatorText("Lootbag...");

      const int columns = inv_x;
      ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchSame;
      ImGui::BeginTable("backpack", columns, table_flags);

      for (const auto& inv_e : inv_c.inv) {
        const auto eid = static_cast<uint32_t>(inv_e);
        ImGui::PushID(eid);
        ImGui::TableNextColumn();

        display_inventory_slot(r, inv_e, button_size);

        // warning: approach does not work with multiple players
        bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
        // clicked |= ImGui::IsItemClicked();

        if (clicked) {
          const auto& slot_c = r.get<InventorySlotComponent>(inv_e);
          const auto item_e = slot_c.item_e;

          // move item to player inventory
          if (item_e != entt::null) {
            auto& player_inv = r.get<DefaultInventory>(e);
            for (const auto& player_inv_slot_e : player_inv.inv) {
              auto& player_slot_c = r.get<InventorySlotComponent>(player_inv_slot_e);
              if (player_slot_c.item_e == entt::null) // free slot
                update_item_parent(r, item_e, player_inv_slot_e);
            }
          }
        }

        ImGui::PopID();
      }

      ImGui::EndTable();
    }
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

} // namespace game2d