#include "ui_lootbag_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/event_coll_player_item/event_coll_player_item_helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"
#include "modules/ui_lootbag/ui_lootbag_helpers.hpp"
#include "ui_lootbag_components.hpp"

#include <imgui.h>

namespace game2d {

void
toggle_lootbag_display(entt::registry& r)
{
  const auto& input = get_first_component<SINGLE_InputComponent>(r);

  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_O)) {
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
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  // toggle_lootbag_display(r);
  // no request: do not show loot menu
  // if (get_first<ShowLootbagRequest>(r) == entt::null)
  //   return;

  auto& ui = get_first_component<SINGLE_UI_Lootbag>(r);

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  const auto viewport_right = viewport_pos.x + ri.viewport_size_render_at.x;
  const auto viewport_top = viewport_pos.y;

  // configs
  const int inv_x = 6;
  const ImVec2 button_size = ImVec2(32, 32); // make the border 48 or 64
  const auto window_0_size = ImVec2{ ri.viewport_size_render_at.x / 6.0f, ri.viewport_size_render_at.y / 3.0f };
  const auto window_0_pos = ImVec2(viewport_right - window_0_size.x, viewport_top + 2.0f * window_0_size.y);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;

  ImGui::SetNextWindowPos(window_0_pos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSizeConstraints(window_0_size, window_0_size);
  ImGui::PushStyleVar(ImGuiTableColumnFlags_WidthFixed, button_size.x);

  ImGui::Begin("UILootbag", NULL, flags);
  ui.hovered = ImGui::IsWindowHovered();

  ImGui::SeparatorText("Loot");
  // ImGui::Text("Right click to take loot");

  const auto& view_players = r.view<PlayerComponent>();
  // ImGui::Text("players: %zu", view_players.size());

  for (const auto& [e, player_c] : view_players.each()) {
    const auto player_eid = static_cast<uint32_t>(e);
    ImGui::PushID(player_eid);

    auto* coll_info = r.try_get<CollInfo>(e);
    if (coll_info == nullptr) {
      ImGui::PopID(); // player eid
      continue;
    }
    // ImGui::Text("colls: %zu", coll_info->other.size());

    for (const auto other_e : coll_info->other) {
      if (e == other_e)
        continue; // player collided with self?

      const auto* inv_c = r.try_get<DefaultInventory>(other_e);
      if (inv_c == nullptr)
        continue;

      if (inv_is_empty(r, *inv_c)) {
        ImGui::SeparatorText("Lootbag... (empty)");
        continue;
      }

      ImGui::SeparatorText("Lootbag...");

      const int columns = inv_x;
      ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchSame;

      std::string label = "backpack##" + std::to_string(player_eid);
      ImGui::BeginTable(label.c_str(), columns, table_flags);

      for (const auto& inv_e : inv_c->inv) {
        const auto eid = static_cast<uint32_t>(inv_e);
        ImGui::PushID(eid);
        ImGui::TableNextColumn();
        display_inventory_slot(r, e, inv_e, button_size);

        // // warning: approach does not work with multiple players
        // bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
        // // clicked |= ImGui::IsItemClicked();

        // if (clicked) {
        //   const auto& slot_c = r.get<InventorySlotComponent>(inv_e);
        //   const auto item_e = slot_c.item_e;

        //   // move item to player inventory
        //   if (item_e != entt::null) {
        //     auto& player_inv = r.get<DefaultInventory>(e);
        //     for (const auto& player_inv_slot_e : player_inv.inv) {
        //       auto& player_slot_c = r.get<InventorySlotComponent>(player_inv_slot_e);
        //       if (player_slot_c.item_e == entt::null) // free slot
        //         update_item_parent(r, item_e, player_inv_slot_e);
        //     }
        //   }
        // }

        ImGui::PopID(); // inventory eid
      }

      ImGui::EndTable();
    }

    ImGui::PopID(); // player eid
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

} // namespace game2d