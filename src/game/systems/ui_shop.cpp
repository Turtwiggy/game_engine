#include "ui_shop.hpp"

#include "game/create_entities.hpp"
#include "game/helpers/items/potion.hpp"

#include <imgui.h>
#include <memory>

namespace game2d {

void
update_ui_shop_system(entt::registry& r)
{
  ImGui::Begin("Shop");

  ImGui::Text("Potion");

  if (ImGui::Button("Buy")) {
    const auto& view = r.view<PlayerComponent>();
    if (view.size() > 0) {
      auto main_player = view.front();
      auto& player = r.get<PlayerComponent>(main_player);

      player.inventory.push_back(std::make_shared<Potion>());
    }
  }

  ImGui::End();
};

} // namespace game2d
