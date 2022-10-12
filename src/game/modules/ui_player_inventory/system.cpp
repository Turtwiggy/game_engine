#include "system.hpp"

#include "game/components/actors.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "modules/renderer/components.hpp"

#include <entt/entt.hpp>
#include <imgui.h>

namespace game2d {

void
game2d::update_ui_player_inventory_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  //
  // Budget Shop
  //

  ImGui::Begin("Shop(s)");
  {
    const auto& shops = r.view<ShopKeeperComponent>();
    const auto& items = r.view<InBackpackComponent, const TagComponent>();

    for (auto [entity_shopkeeper, shopkeeper] : shops.each()) {
      for (auto [entity_item, backpack, tag] : items.each()) {

        if (backpack.parent != entity_shopkeeper)
          continue; // not shopkeepers item

        ImGui::Text("Buy: %s", tag.tag.c_str());
        std::string label_buy = "Buy##" + std::to_string(static_cast<uint32_t>(entity_item));

        // Hack: buy for the first player at the moment
        const auto& players = r.view<PlayerComponent>();
        const auto& player0 = players.front();

        ImGui::SameLine();
        if (ImGui::Button(label_buy.c_str())) {
          auto& purchase = r.get_or_emplace<WantsToPurchase>(player0);
          purchase.items.push_back(entity_item);
        }
      }
    }
  }
  ImGui::End();

  //
  // Budget Inventory Display
  //

  ImGui::Begin("Player(s) UI");
  {
    const auto& players = r.view<const PlayerComponent>();
    const auto& items = r.view<const InBackpackComponent, const TagComponent>();

    for (auto [entity_player, player] : players.each()) {

      // Hp..!
      if (auto* hp = r.try_get<HealthComponent>(entity_player))
        ImGui::Text("Hp: %i", hp->hp);

      // Inventory..!
      ImGui::Text("¬¬ Inventory ¬¬");

      for (auto [entity_item, backpack, tag] : items.each()) {

        if (backpack.parent != entity_player)
          continue; // not my item

        ImGui::Text("%s", tag.tag.c_str());

        ImGui::SameLine();
        std::string label_use = "Use##" + std::to_string(static_cast<uint32_t>(entity_item));
        if (ImGui::Button(label_use.c_str())) {
          auto& u = r.get_or_emplace<WantsToUse>(entity_player);
          Use info;
          info.entity = entity_item;
          info.targets = { entity_player }; // self
          u.items.push_back(info);
        }

        ImGui::SameLine();
        std::string label_drop = "Drop##" + std::to_string(static_cast<uint32_t>(entity_item));
        if (ImGui::Button(label_drop.c_str())) {
          auto& u = r.get_or_emplace<WantsToDrop>(entity_player);
          u.items.push_back(entity_item);
        }

        ImGui::SameLine();
        std::string label_sell = "Sell##" + std::to_string(static_cast<uint32_t>(entity_item));
        if (ImGui::Button(label_sell.c_str())) {
          auto& u = r.get_or_emplace<WantsToSell>(entity_player);
          u.items.push_back(entity_item);
        }
      }
    }
  }
  ImGui::End();
};

} // namespace game2d