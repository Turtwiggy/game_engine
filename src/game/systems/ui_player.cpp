#include "ui_player.hpp"

#include "game/components/breakable.hpp"
#include "game/create_entities.hpp"
#include "game/helpers/items.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <vector>

namespace game2d {

void
update_ui_player_system(entt::registry& r)
{
  ImGui::Begin("Player");

  int i = 0; // each item requires unique id

  const auto& view = r.view<PlayerComponent, BreakableComponent>();
  view.each([&r, &i](auto entity, auto& player, auto& hp) {
    //
    ImGui::Text("¬¬Player¬¬");
    ImGui::Text("HP: %i", hp.hp);

    ImGui::Text("¬¬Equipment¬¬");

    ImGui::Text("Head");
    if (player.head == nullptr)
      ImGui::Text("¬empty¬");
    else
      ImGui::Text("%s", player.head.get()->name.c_str());

    ImGui::Text("Body");
    if (player.body == nullptr)
      ImGui::Text("¬empty¬");
    else
      ImGui::Text("%s", player.body.get()->name.c_str());

    ImGui::Text("Hand (L)");
    if (player.hand_l == nullptr)
      ImGui::Text("¬empty¬");
    else
      ImGui::Text("%s", player.hand_l.get()->name.c_str());

    ImGui::Text("Hand (R)");
    if (player.hand_r == nullptr)
      ImGui::Text("¬empty¬");
    else
      ImGui::Text("%s", player.hand_r.get()->name.c_str());

    ImGui::Text("Inventory");
    if (auto* inventory = r.try_get<InventoryComponent>(entity)) {
      auto& inv = (*inventory);
      std::vector<std::shared_ptr<Equipment>>::iterator it = inv.inventory.begin();
      while (it != inv.inventory.end()) {
        ImGui::Text("Item: %s", (*it)->name.c_str());
        ImGui::SameLine();

        std::string tag = std::string("Equip##") + std::to_string(i);
        i++; // imgui requires unique button names

        if (ImGui::Button(tag.c_str())) {
          if (player.hand_l == nullptr) {
            // move to player's hand
            player.hand_l = (*it);
            it = inv.inventory.erase(it);
            continue;
          } else {
            // swap equipped with inventory items
            std::shared_ptr<Equipment> equipped = player.hand_l;
            player.hand_l = (*it);
            it->swap(equipped);
          }
        }

        ++it;
      }
    }
  });

  ImGui::End();
}

} // namespace game2d