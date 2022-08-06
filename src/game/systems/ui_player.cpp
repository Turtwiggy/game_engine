#include "ui_player.hpp"

#include "game/components/breakable.hpp"
#include "game/create_entities.hpp"
#include "game/helpers/items.hpp"

#include <imgui.h>

#include <vector>

namespace game2d {

void
update_ui_player_system(entt::registry& r)
{
  ImGui::Begin("Player");

  const auto& view = r.view<PlayerComponent, BreakableComponent>();
  view.each([&r](auto entity, auto& player, auto& hp) {
    //
    ImGui::Text("¬¬Player¬¬");
    ImGui::Text("HP: %i", hp.hp);

    ImGui::Text("Active Equipment");
    ImGui::Text("Head");
    ImGui::Text("¬empty¬");
    ImGui::Text("Body");
    ImGui::Text("¬empty¬");
    ImGui::Text("Hand (L)");
    ImGui::Text("¬empty¬");
    ImGui::Text("Hand (R)");
    ImGui::Text("¬empty¬");
    ImGui::Text("Legs");
    ImGui::Text("¬empty¬");
    ImGui::Text("Feet");
    ImGui::Text("¬empty¬");

    ImGui::Text("Backpack");
    if (auto* inventory = r.try_get<InventoryComponent>(entity)) {
      ImGui::Text("(infinite) default-inventory");

      auto& inv = (*inventory);
      auto it = inv.inventory.begin();

      int i = 0;

      while (it != inv.inventory.end()) {
        ImGui::Text("Item: %s", (*it)->name.c_str());
        ImGui::SameLine();

        std::string tag = std::string("Use##") + std::to_string(i);
        i++; // imgui requires unique button names

        if (ImGui::Button(tag.c_str())) {
          std::vector<entt::entity> entities_to_apply_to = { entity };
          if ((*it)->use(r, entities_to_apply_to)) {
            if ((*it)->destroy_after_use) {
              if ((*it)->count > 1) {
                (*it)->count -= 1;
              } else {
                it = inv.inventory.erase(it);
                continue;
              }
            }
          }
        }
        ++it;
      }
    } else {
      ImGui::Text("¬empty¬");
    }
  });

  ImGui::End();
}

} // namespace game2d