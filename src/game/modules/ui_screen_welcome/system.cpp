#include "system.hpp"

#include "game/modules/player/components.hpp"
#include "game/modules/ui_screen_gameover/helpers.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace game2d {

void
update_ui_welcome_system(GameEditor& editor, Game& game)
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

  if (game.running_state == GameState::START) {
    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(
      ImVec2(io.DisplaySize.x * 0.25f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    bool show = true;
    ImGui::Begin("Welcome", &show, flags);

    ImGui::Text("Welcome!");
    ImGui::Text("");
    ImGui::Text("¬¬ MOVE ¬¬");
    ImGui::Text("Move with WASD.");
    ImGui::Text("");
    ImGui::Text("¬¬ ATTACK ¬¬");
    ImGui::Text("Collide with enemy to damage.");
    ImGui::Text("");
    ImGui::Text("¬¬ LEVEL STATS ¬¬");
    ImGui::Text("CON: +1 MAX HP");
    ImGui::Text("STR: +1 ATK");
    ImGui::Text("AGI: +1 DEF");
    ImGui::Text("");
    ImGui::Text("¬¬ ITEMS ¬¬");
    ImGui::Text("sword: +1 ATK");
    ImGui::Text("shield: +1 DEF");
    ImGui::Text("potion: +1 HP");

    ImGui::Text("");
    ImGui::Text("Dungeon Seed:");
    static int seed = 2;
    if (ImGui::InputInt("Seed", &seed)) {
      game.live_dungeon_seed = glm::abs(seed);
    }

    ImGui::Text("");
    if (ImGui::Button("Begin Adventure!")) {
      std::cout << "\nrestart game button clicked\n";
      restart_game(editor, game, seed);
      game.running_state = GameState::RUNNING;
    }

    ImGui::End();
  }
};

} // namespace game2d