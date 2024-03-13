#include "system.hpp"

#include "actors.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"

#include "imgui.h"
#include "magic_enum.hpp"

namespace game2d {

void
update_ui_xp_bar_system(entt::registry& r)
{
  ImGui::Begin("XP");

  const auto& player_view = r.view<const PlayerComponent, const HealthComponent>(entt::exclude<WaitForInitComponent>);
  const auto& items_view =
    r.view<const ItemComponent, const HasParentComponent, const EntityTypeComponent>(entt::exclude<WaitForInitComponent>);

  for (const auto& [player_e, player_c, player_hp] : player_view.each()) {
    const auto eid = static_cast<uint32_t>(player_e);
    ImGui::PushID(eid);

    // draw a healthbar
    //
    const float hp_percent = player_hp.hp / (float)player_hp.max_hp;

    // draw an xp bar
    //
    float xp_percent = 0.0f;
    int xp = 0;
    const int xp_total = 10;
    {
      // Show like potion x1, potion x2 not potions individually
      std::map<EntityType, std::vector<entt::entity>> compacted_items;
      for (const auto& [item_e, item_c, item_parent, item_type] : items_view.each()) {
        if (item_parent.parent != player_e)
          continue; // not my item
        compacted_items[item_type.type].push_back(item_e);
      }

      // count xp
      if (compacted_items.find(EntityType::actor_pickup_xp) != compacted_items.end())
        xp = compacted_items[EntityType::actor_pickup_xp].size();
      xp_percent = xp / (float)xp_total;
    }

    //
    // draw things
    //
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // healthbar
    {
      ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
      ImVec2 hp_canvas_sz = ImGui::GetContentRegionAvail();
      hp_canvas_sz.x *= hp_percent;
      hp_canvas_sz.y = 12.0f;
      ImVec2 canvas_p1 = ImVec2(canvas_p0.x + hp_canvas_sz.x, canvas_p0.y + hp_canvas_sz.y);
      draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(200, 25, 25, 255));
      ImGui::Text("%i/%i", player_hp.hp, player_hp.max_hp);
    }

    // xpbar
    {
      ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
      ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
      canvas_sz.x *= xp_percent;
      canvas_sz.y = 12.0f;
      ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
      draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(25, 200, 150, 255));
      ImGui::Text("%i/%i", xp, xp_total);
    }

    ImGui::PopID();

    break; // work how how to do multiple players
  }

  ImGui::End();
}

} // namespace game2d