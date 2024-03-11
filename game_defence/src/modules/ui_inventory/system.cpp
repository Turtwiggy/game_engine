#include "system.hpp"

#include "entt/helpers.hpp"
#include "imgui/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_powerup_doubledamage/components.hpp" // should be removed
#include "modules/items/helpers.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lifecycle/components.hpp"

#include "imgui.h"
#include "magic_enum.hpp"

#include <string>

namespace game2d {
using namespace std::literals;

void
update_ui_inventory(entt::registry& r)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  ImGui::Begin("Inventory");
  {
    const auto& players_view = r.view<PlayerComponent>(entt::exclude<WaitForInitComponent>);
    const auto& items_view = r.view<ItemComponent, HasParentComponent, TagComponent>(entt::exclude<WaitForInitComponent>);

    for (const auto& [player_e, player_c] : players_view.each()) {
      const auto eid = static_cast<uint32_t>(player_e);
      ImGui::PushID(eid);
      ImGui::Text("¬¬ Player ¬¬");
      ImGui::Text("Player has %i XP", player_c.picked_up_xp);
      ImGui::Text("Player has doubledamage: %i", r.try_get<PowerupDoubleDamage>(player_e) != nullptr);

      // HACK: give player xp
      if (ImGui::Button("Give XP")) {
        player_c.picked_up_xp += 1;
      }

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
        const auto entity_type = r.get<EntityTypeComponent>(entity_item).type;
        const auto entity_type_name = std::string(magic_enum::enum_name(entity_type));

        //
        ImGui::Text("Player has %s (%i times)", entity_type_name.c_str(), entity_items.size());
      }

      ImGui::Separator();
      ImGui::Text("¬¬ Player Upgrades ¬¬");
      ImGui::Text("TODO");

      ImGui::PopID();
    }
  }
  ImGui::End();
}

} // namespace game2d