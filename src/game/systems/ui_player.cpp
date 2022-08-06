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

    if (auto* inventory = r.try_get<InventoryComponent>(entity)) {
      ImGui::Text("¬Inventory¬");

      auto& inv = (*inventory);
      std::vector<std::shared_ptr<Item>>::iterator it = inv.inventory.begin();

      while (it != inv.inventory.end()) {
        ImGui::Text("Item: %s", (*it)->name.c_str());
        ImGui::SameLine();

        if (ImGui::Button("Use")) {
          std::vector<entt::entity> entities_to_apply_to = { entity };
          if ((*it)->use(r, entities_to_apply_to)) {
            it = inv.inventory.erase(it);
            continue;
          }
        }
        ++it;
      }
    } else {
      ImGui::Text("¬No Inventory¬");
    }
  });

  ImGui::End();
}

} // namespace game2d