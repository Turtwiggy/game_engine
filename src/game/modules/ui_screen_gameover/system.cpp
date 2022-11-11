#include "system.hpp"

#include "game/modules/player/components.hpp"
#include "game/modules/ui_screen_gameover/helpers.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_gameover_system(GameEditor& editor, Game& game)
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

  if (game.running_state == GameState::GAMEOVER_LOSE || game.running_state == GameState::GAMEOVER_WIN) {
    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(
      ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    bool show = true;
    ImGui::Begin("Gameover", &show, flags);

    if (game.running_state == GameState::GAMEOVER_LOSE)
      ImGui::Text("Dead!!");
    if (game.running_state == GameState::GAMEOVER_WIN)
      ImGui::Text("You won!!");
    ImGui::Text("");

    ImGui::Text("Thanks for playing.");
    ImGui::Text("");
    ImGui::Text("Feedback appreciated at");
    ImGui::Text("Tweet @MarkBerrow");
    ImGui::Text("Discord Turtwiggy#5041");

    ImGui::Separator();
    if (ImGui::Button("Restart"))
      game.running_state = GameState::START;

    ImGui::End();
  }
}

} // namespace game2d