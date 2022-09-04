#include "ui_player_inventory.hpp"

#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "modules/renderer/components.hpp"

#include <imgui.h>

namespace game2d {

void
game2d::update_ui_player_inventory_system(entt::registry& r)
{
  //
  // Budget Shop
  //

  ImGui::Begin("Shop(s)");
  {
    const auto& shops = r.view<ShopKeeperComponent>();
    shops.each([&r](auto entity_shopkeeper, auto& shopkeeper) {
      {
        const auto& items = r.view<InBackpackComponent, const TagComponent>();
        items.each([&r, &entity_shopkeeper](entt::entity item, auto& item_comp, const auto& tag) {
          if (item_comp.parent != entity_shopkeeper)
            return; // not my item

          ImGui::Text("Buy: %s", tag.tag.c_str());
          std::string label_buy = "Buy##" + std::to_string(static_cast<uint32_t>(item));

          // Hack: buy for the first player at the moment
          const auto& players = r.view<PlayerComponent>();
          const auto& player0 = players.front();

          ImGui::SameLine();
          if (ImGui::Button(label_buy.c_str())) {
            auto& items = r.get_or_emplace<WantsToPurchase>(player0);
            items.items.push_back(item);
          }
        });
      }
    });
  }
  ImGui::End();

  //
  // Budget Inventory Display
  //

  ImGui::Begin("Player(s) UI");
  {
    const auto& view = r.view<PlayerComponent>();
    view.each([&r](auto entity_player, auto& player) {
      // Hp..!
      if (auto* hp = r.try_get<HealthComponent>(entity_player))
        ImGui::Text("Hp: %i", hp->hp);

      // Inventory..!
      ImGui::Text("¬¬ Inventory ¬¬");

      const auto& items = r.view<InBackpackComponent, const TagComponent>();
      items.each([&r, &entity_player](entt::entity item, auto& item_comp, const auto& tag) {
        if (item_comp.parent != entity_player)
          return; // not my item

        ImGui::Text("%s", tag.tag.c_str());
        std::string label_use = "Use##" + std::to_string(static_cast<uint32_t>(item));
        // std::string label_equip = "Equip##" + std::to_string(static_cast<uint32_t>(item));
        std::string label_drop = "Drop##" + std::to_string(static_cast<uint32_t>(item));
        std::string label_sell = "Sell##" + std::to_string(static_cast<uint32_t>(item));

        ImGui::SameLine();
        if (ImGui::Button(label_use.c_str())) {
          auto& items = r.get_or_emplace<WantsToUse>(entity_player);

          Use info;
          info.item = item;
          info.targets = { entity_player }; // self

          items.items.push_back(info);
        }

        // ImGui::SameLine();
        // if (ImGui::Button(label_equip.c_str()))
        //   std::cout << "You want to equip an inventory item " << "\n";

        ImGui::SameLine();
        if (ImGui::Button(label_drop.c_str())) {
          auto& items = r.get_or_emplace<WantsToDrop>(entity_player);
          items.items.push_back(item);
        }

        ImGui::SameLine();
        if (ImGui::Button(label_sell.c_str())) {
          auto& items = r.get_or_emplace<WantsToSell>(entity_player);
          items.items.push_back(item);
        }
      });
    });
  }
  ImGui::End();
};

} // namespace game2d