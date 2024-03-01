#include "system.hpp"

#include "entt/helpers.hpp"
#include "imgui/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/items/helpers.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lifecycle/components.hpp"

#include "imgui.h"

#include <string>

namespace game2d {
using namespace std::literals;

void
update_ui_inventory(entt::registry& r)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  ImGui::Begin("Inventory");
  {
    const auto& players_view = r.view<const PlayerComponent>(entt::exclude<WaitForInitComponent>);
    const auto& items_view = r.view<ItemComponent, HasParentComponent, TagComponent>(entt::exclude<WaitForInitComponent>);

    for (const auto& [player_e, player_c] : players_view.each()) {
      const auto eid = static_cast<uint32_t>(player_e);
      ImGui::PushID(eid);
      ImGui::Text("¬¬ Player ¬¬");
      ImGui::Text("Player has %i XP", player_c.picked_up_xp);
      ImGui::Text("Player has %i kills", player_c.killed);

      // Show like potion x1, potion x2 not potions individually
      std::map<std::string, std::vector<entt::entity>> compacted_items;
      for (const auto& [item_e, item_c, item_parent, item_tag] : items_view.each()) {
        if (item_parent.parent != player_e)
          continue; // not my item
        compacted_items[item_tag.tag].push_back(item_e);
      }

      for (const auto& [tag, entity_items] : compacted_items) {

        // assume all the compacted items have the same state
        // this could break if potions had internal state,
        // e.g. usages left
        const auto& entity_item = entity_items[0];

        //
        ImGui::Text("Player has X Item... (%i times)", entity_items.size());
      }

      ImGui::PopID();
    }
  }
  ImGui::End();
}

} // namespace game2d