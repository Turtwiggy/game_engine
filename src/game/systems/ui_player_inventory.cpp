#include "ui_player_inventory.hpp"

#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "modules/renderer/components.hpp"

#include <imgui.h>

namespace game2d {

void
game2d::update_ui_player_inventory_system(entt::registry& r)
{
#ifdef _DEBUG
  // bool show_imgui_demo_window = false;
  // ImGui::ShowDemoWindow(&show_imgui_demo_window);
#endif

  //
  // Budget Shop
  // Note: anything that changes game state should be done in fixed_update()
  //

  ImGui::Begin("Player(s) Shop");
  {
    int i = 0;
    const auto& view = r.view<PlayerComponent, HealthComponent>();
    view.each([&r, &i](auto entity, auto& player, const auto& hp) {
      ImGui::Text("Player %i", i++);
      ImGui::Text("(debug) Current HP: %i", hp.hp);
      if (ImGui::Button("Buy Potion"))
        create_item(r, ENTITY_TYPE::POTION, entity);
      if (ImGui::Button("Buy Crossbow"))
        create_item(r, ENTITY_TYPE::CROSSBOW, entity);
    });
  }
  ImGui::End();

  //
  // Budget Inventory Display
  //

  ImGui::Begin("Player(s) Inventory");
  {
    const auto& view = r.view<PlayerComponent>();
    view.each([&r](auto entity, auto& player) {
      {
        const auto& items = r.view<InBackpackComponent, const TagComponent>();
        items.each([&r, &entity](entt::entity item, auto& item_comp, const auto& tag) {
          {
            if (item_comp.parent == entity) {
              ImGui::Text("%s", tag.tag.c_str());

              ImGui::SameLine();
              std::string label = "Use##" + std::to_string(static_cast<uint32_t>(item));
              if (ImGui::Button(label.c_str())) {
                std::cout << "(warning, attaching drink component for all items) -- You want to use an inventory item "
                          << std::endl;

                // warning, currently emplacing for all items not just potion item
                r.emplace_or_replace<WantsToDrinkPotionComponent>(entity, item);
                r.remove<InBackpackComponent>(item);
              }

              ImGui::SameLine();
              label = "Equip##" + std::to_string(static_cast<uint32_t>(item));
              if (ImGui::Button(label.c_str())) {
                std::cout << "You want to equip an inventory item " << std::endl;
              }
            }
          }
        });
      }
    });
  }
  ImGui::End();
};

} // namespace game2d