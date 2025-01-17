#include "ui_inventory_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"

#include "imgui.h"
#include <magic_enum.hpp>

#include <SDL2/SDL_log.h>

namespace game2d {

void
update_ui_inventory_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  // #if defined(_DEBUG)
  //   static bool first_time = false;
  //   if (first_time) {
  //     first_time = false;
  //     destroy_first_and_create<ShowInventoryRequest>(r);
  //   }
  // #endif
  // toggle_inventory_display(r);

  const auto& view = r.view<PlayerComponent, DefaultBody, DefaultInventory>();
  for (const auto& [e, player_c, body_c, inv_c] : view.each()) {
    //
    update_initialize_inventory(r, e);

    // Four cases to handle here.
    // UI: yes. Instance: yes. all good.
    // UI: yes. Instance: no. spawn.
    // UI: no. Instance: no. all good.
    // UI: no. Instance: yes. destroy.

    const auto gun_slot_e = get_slot_type(r, body_c.body, InventorySlotType::weapon);
    const auto& gun_slot_c = r.get<InventorySlotComponent>(gun_slot_e);
    const auto* weapon_c = r.try_get<HasWeaponComponent>(e);

    // There's an item in the gunslot... but no weapon...
    if (gun_slot_c.item_e != entt::null && !weapon_c) {
      auto& ui_gun = r.get<UI_ItemComponent>(gun_slot_c.item_e);

      auto& tag = r.get<TagComponent>(gun_slot_c.item_e);
      // if (tag.tag.find("shotgun") != std::string::npos)
      //   const auto shotgun_e = create_shotgun(r, e); // adds an HasWeaponComponent to the player_e

      continue; // spawn gun
    }

    // There's no item in the gunslot..
    if (gun_slot_c.item_e == entt::null && weapon_c) {
      auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
      dead.dead.emplace(weapon_c->instance);
      r.remove<HasWeaponComponent>(e);
      continue; // delete gun
    }

    // there's an item in the gunslot and an instance is spawned...
    if (gun_slot_c.item_e != entt::null && weapon_c) {
      const auto& a_tag = r.get<TagComponent>(gun_slot_c.item_e).tag;
      const auto& b_tag = r.get<TagComponent>(weapon_c->instance).tag;
      // ImGui::Text("a_tag: %s, b_tag: %s", a_tag.c_str(), b_tag.c_str());
      if (a_tag != b_tag) {
        // destroy the instance
        auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
        dead.dead.emplace(weapon_c->instance);
        r.remove<HasWeaponComponent>(e);
      }
    }
  }

  // no request: do not show inventory
  // if (get_first<ShowInventoryRequest>(r) == entt::null)
  //   return;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;

  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  const auto viewport_right = viewport_pos.x + ri.viewport_size_render_at.x;
  const auto viewport_top = viewport_pos.y;

  // configs
  const int inv_x = 6;
  const auto button_size = ImVec2(32, 32); // make the border 48 or 64
  const auto window_0_size = ImVec2{ ri.viewport_size_render_at.x / 6.0f, ri.viewport_size_render_at.y / 3.0f };
  const auto window_1_size = ImVec2{ ri.viewport_size_render_at.x / 6.0f, ri.viewport_size_render_at.y / 3.0f };
  const auto window_0_pos = ImVec2(viewport_right - window_0_size.x, viewport_top);
  const auto window_1_pos = ImVec2(viewport_right - window_1_size.x, viewport_top + window_0_size.y);

  ImGui::SetNextWindowPos(window_0_pos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSizeConstraints(window_0_size, window_0_size);
  ImGui::Begin("Inventory-Equipment", NULL, flags);
  ImGui::SeparatorText("Equipment");

  // Scrollable content
  float scroll_height = ImGui::GetContentRegionAvail().y;
  ImGui::BeginChild("ScrollableRegion", ImVec2(0, scroll_height), true, ImGuiWindowFlags_HorizontalScrollbar);

  const auto& body_view = r.view<PlayerComponent, DefaultBody, const SelectedComponent>();
  for (const auto& [e, player_c, body_c, selected_c] : body_view.each()) {
    for (size_t i = 0; i < body_c.body.size(); i++) {
      const auto eid = static_cast<uint32_t>(body_c.body[i]);
      ImGui::PushID(eid);
      display_inventory_slot(r, e, body_c.body[i], button_size);
      ImGui::PopID();
    }
  }

  ImGui::EndChild();
  ImGui::End();

  ImGui::SetNextWindowPos(window_1_pos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSizeConstraints(window_1_size, window_1_size);
  ImGui::PushStyleVar(ImGuiTableColumnFlags_WidthFixed, button_size.x);
  ImGui::Begin("Inventory-Backpack", NULL, flags);
  ImGui::SeparatorText("Inventory");

  // Scrollable content
  scroll_height = ImGui::GetContentRegionAvail().y;
  ImGui::BeginChild("ScrollableRegion", ImVec2(0, scroll_height), true, ImGuiWindowFlags_HorizontalScrollbar);

  const auto& inv_view = r.view<PlayerComponent, DefaultInventory, const SelectedComponent>();
  int inv_units = 0;

  for (int i = 0; const auto& [e, player_c, inv_c, selected_c] : inv_view.each()) {
    for (const auto& inv_e : inv_c.inv) {
      const auto eid = static_cast<uint32_t>(inv_e);
      ImGui::PushID(eid);
      display_inventory_slot(r, e, inv_e, button_size);
      ImGui::PopID();
    }
    inv_units++;
  }

  if (inv_units == 0)
    ImGui::Text("No player selected.");

  ImGui::EndChild();
  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
}

} // namespace game2d