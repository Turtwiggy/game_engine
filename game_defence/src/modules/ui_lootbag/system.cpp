#include "system.hpp"

#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "imgui.h"
#include "maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"

namespace game2d {

void
update_ui_lootbag_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = get_first_component<MapComponent>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

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

  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 0.5f));
  ImGui::SetNextWindowSizeConstraints(window_0_size, window_0_size);
  ImGui::PushStyleVar(ImGuiTableColumnFlags_WidthFixed, button_size.x);
  ImGui::Begin("UILootbag", NULL, flags);
  ImGui::SeparatorText("Loot");
  const auto content_size = ImGui::GetContentRegionAvail();

  for (const auto& [e, player_c] : r.view<PlayerComponent>().each()) {
    const auto gp = get_grid_position(r, e);
    const auto idx = engine::grid::grid_position_to_index(gp, map_c.xmax);

    // remove the player from the grid cell index,
    // and assume that every other entity is an inventory.
    auto es = map_c.map[idx]; // take a copy
    auto it = std::remove(es.begin(), es.end(), e);
    if (it != es.end())
      es.erase(it, es.end());

    const std::string info_str = std::format("Standing on {} lootbags...", es.size());
    ImGui::Text("%s", info_str.c_str());

    // should be all inventories
    for (const auto inv_e : es) {
      ImGui::SeparatorText("Lootbag...");

      const int columns = inv_x;
      ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchSame;
      ImGui::BeginTable("backpack", columns, table_flags);

      const auto& inv_c = r.get<DefaultInventory>(inv_e);

      for (const auto& inv_e : inv_c.inv) {
        const auto eid = static_cast<uint32_t>(inv_e);
        ImGui::PushID(eid);
        ImGui::TableNextColumn();
        display_inventory_slot(r, inv_e, button_size);
        ImGui::PopID();
      }

      ImGui::EndTable();
    }
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

} // namespace game2d