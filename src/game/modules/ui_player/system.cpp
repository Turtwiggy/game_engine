#include "system.hpp"

#include "game/modules/player/components.hpp"
#include "game/modules/rpg/components.hpp"

#include <imgui.h>

namespace game2d {

void
game2d::update_ui_player_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  ImGui::Begin("Player UI");

  // Player Info
  for (auto [entity, player] : r.view<PlayerComponent>().each()) {
    ImGui::Text("Player X in game");
  }

  // XP info
  for (auto [entity, player, levelup] : r.view<PlayerComponent, WantsToLevelUp>().each()) {
    ImGui::Text("Player needs leveling up!");

    if (ImGui::Button("Level up player X")) {
      // TODO: level up
      game.ui_events.events.push_back("Boom! leveld up");

      r.remove<WantsToLevelUp>(entity);
    }
  }

  ImGui::End();
}

} // namespace game2d