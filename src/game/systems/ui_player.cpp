#include "ui_player.hpp"

#include "game/create_entities.hpp"
#include "game/helpers/items/item.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_player_system(entt::registry& r)
{
  ImGui::Begin("Player");

  const auto& view = r.view<PlayerComponent>();
  view.each([&r](auto entity, auto& player) {
    //
    //
    //
    ImGui::Text("¬¬Player¬¬");
    ImGui::Text("¬Inventory¬");

    for (int i = 0; i < player.inventory.size(); i++) {
      ImGui::Text("Item: %s", player.inventory[i]->name.c_str());
      if (ImGui::Button("Use"))
        if (player.inventory[i]->use(r)) {
          // remove it?
        }
    }
  });

  ImGui::End();
}

} // namespace game2d