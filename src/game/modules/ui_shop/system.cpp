#include "system.hpp"

#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/helpers/check_equipment.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "modules/renderer/components.hpp"

#include "magic_enum.hpp"
#include <entt/entt.hpp>
#include <imgui.h>

#include <map>
#include <string>

namespace game2d {

void
game2d::update_ui_shop_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  //
  // Budget Shop
  //

  static bool shop_open = false;

  if (shop_open) {
    ImGui::SetNextWindowPos(ImVec2{ 100, 100 }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2{ 100, 100 }, ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoFocusOnAppearing;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoResize;
    ImGui::Begin("(debug) Shop", &shop_open, flags);
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
  }
}

} // namespace game2d