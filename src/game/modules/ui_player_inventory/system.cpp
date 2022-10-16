#include "system.hpp"

#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/helpers/distance.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ux_hover/components.hpp"

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

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  static bool shop_open = true;

  ImGui::SetNextWindowPos(ImVec2{ 100, 100 }, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2{ 100, 100 }, ImGuiCond_FirstUseEver);
  ImGui::Begin("Shop", &shop_open, flags);
  {
    const auto& shops = r.view<ShopKeeperComponent>();
    const auto& items = r.view<InBackpackComponent, const TagComponent>();

    for (auto [entity_shopkeeper, shopkeeper] : shops.each()) {
      for (auto [entity_item, backpack, tag] : items.each()) {

        if (backpack.parent != entity_shopkeeper)
          continue; // not shopkeepers item

        // Hack: buy for the first player at the moment
        const auto& players = r.view<PlayerComponent>();
        const auto& player0 = players.front();

        std::string label_buy = "Buy##" + std::to_string(static_cast<uint32_t>(entity_item));

        if (ImGui::Button(label_buy.c_str())) {
          auto& purchase = r.get_or_emplace<WantsToPurchase>(player0);
          purchase.items.push_back(entity_item);
        }
        ImGui::SameLine();
        ImGui::Text("%s", tag.tag.c_str());
      }
    }
  }
  ImGui::End();

  //
  // Budget Inventory Display
  //
  const auto& colours = editor.colours;

  static bool player_ui_open = true;
  ImGui::Begin("Player(s) UI", &player_ui_open, flags);
  {
    const auto& players = r.view<const PlayerComponent>();

    for (auto [entity_player, player] : players.each()) {

      // Hp..!
      if (auto* hp = r.try_get<HealthComponent>(entity_player))
        ImGui::Text("Hp: %i", hp->hp);

      // UI: selecting units for an item
      if (auto* selecting = r.try_get<WantsToSelectUnitsForItem>(entity_player)) {
        ImGui::Text("¬¬ Select Targets for Item ¬¬");

        for (auto& item : selecting->items) {
          ImGui::Text("Item: %s", r.get<TagComponent>(item.entity).tag.c_str());
          for (const auto& target : item.targets)
            ImGui::Text("Target: %s", r.get<TagComponent>(target).tag.c_str());

          if (ImGui::Button("Clear Targets")) {
            item.targets.clear();                    // item state
            r.clear<CollidingWithCursorComponent>(); // coll state
          }
          ImGui::Separator();

          std::string label_confirm = "Confirm Use##" + std::to_string(static_cast<uint32_t>(item.entity));
          std::string label_cancel = "Cancel Use##" + std::to_string(static_cast<uint32_t>(item.entity));

          // If confirm: apply item to targets
          if (ImGui::Button(label_confirm.c_str())) {
            {
              auto& u = r.get_or_emplace<WantsToUse>(entity_player);
              Use info;
              info.entity = item.entity;
              for (const auto& target : item.targets)
                info.targets.push_back(target);
              u.items.push_back(info);
            }
            r.remove<WantsToSelectUnitsForItem>(entity_player);
          }
          // If cancel: dont select any units
          else if (ImGui::Button(label_cancel.c_str())) {
            r.remove<WantsToSelectUnitsForItem>(entity_player);
          }
        }
      }

      // Inventory..!
      ImGui::Text("¬¬ Inventory ¬¬");
      const auto& items = r.view<const InBackpackComponent, const TagComponent, const EntityTypeComponent>();
      for (auto [entity_item, backpack, tag, type] : items.each()) {

        if (backpack.parent != entity_player)
          continue; // not my item

        std::string label_use = "Use##" + std::to_string(static_cast<uint32_t>(entity_item));
        if (ImGui::Button(label_use.c_str())) {

          // USE_TYPE: SELF
          if (type.type == EntityType::potion) {
            auto& u = r.get_or_emplace<WantsToUse>(entity_player);
            Use info;
            info.entity = entity_item;
            info.targets = { entity_player };
            u.items.push_back(info);
          }

          // USE_TYPE: NEAREST
          if (type.type == EntityType::scroll_damage_nearest) {
            entt::entity nearest = get_nearest_attackable(game, entity_player);
            auto& u = r.get_or_emplace<WantsToUse>(entity_player);
            Use info;
            info.entity = entity_item;
            if (nearest != entt::null)
              info.targets = { nearest };
            u.items.push_back(info);
          }

          // USE_TYPE: SELECT ENTITIES
          if (type.type == EntityType::scroll_damage_selected_on_grid) {
            auto& a = r.get_or_emplace<WantsToSelectUnitsForItem>(entity_player);
            Use info;
            info.entity = entity_item;
            // info.targets; // unknown until user selects
            a.items.push_back(info);
          }
        }

        ImGui::SameLine();
        std::string label_drop = "Drop##" + std::to_string(static_cast<uint32_t>(entity_item));
        if (ImGui::Button(label_drop.c_str())) {
          auto& u = r.get_or_emplace<WantsToDrop>(entity_player);
          u.items.push_back(entity_item);
        }

        // ImGui::SameLine();
        // std::string label_sell = "Sell##" + std::to_string(static_cast<uint32_t>(entity_item));
        // if (ImGui::Button(label_sell.c_str())) {
        //   auto& u = r.get_or_emplace<WantsToSell>(entity_player);
        //   u.items.push_back(entity_item);
        // }

        // LABEL
        ImGui::SameLine();
        ImGui::Text("%s", tag.tag.c_str());
      }
    }

    // DEBUG: highlight the nearest enemy
    // entt::entity nearest = get_nearest_attackable(game, entity_player);
    // if (nearest != entt::null) {
    //   r.emplace_or_replace<SelectableComponent>(nearest, true);
    //   SpriteColourComponent& sc = r.get<SpriteColourComponent>(nearest);
    //   HoverComponent hc;
    //   hc.regular_colour = engine::LinearToSRGB(sc.colour);
    //   hc.hover_colour = colours.green;
    //   r.emplace_or_replace<HoverComponent>(nearest, hc);
    // }
  }
  ImGui::End();
}

} // namespace game2d