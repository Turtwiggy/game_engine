#include "system.hpp"

#include "game/modules/player/components.hpp"
#include "game/modules/ui_gameover/helpers.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_gameover_system(GameEditor& editor, Game& game, uint32_t& seed)
{
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiDockNodeFlags_AutoHideTabBar;
  flags |= ImGuiDockNodeFlags_NoResize;

  const auto& viewport = ImGui::GetWindowViewport();
  float x = viewport->Pos.x + (viewport->Size.x / 2.0f);
  float y = viewport->Pos.y + (viewport->Size.y / 2.0f);

  if (game.gameover) {
    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(
      ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    bool show = true;
    ImGui::Begin("Gameover", &show, flags);

    auto& r = game.state;
    if (r.view<PlayerComponent>().size() > 0) {
      ImGui::Text("You won!!");
    } else {
      ImGui::Text("Dead-ed!!");
    }

    ImGui::Text("Thanks for playing!");
    ImGui::Text("For suggestions/feedback please tweet @MarkBerrow :)");

    if (ImGui::Button("Restart"))
      restart_game(editor, game, seed);

    ImGui::End();
  }
}
}