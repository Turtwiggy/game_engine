#include "system.hpp"

#include "modules/player/components.hpp"
#include "modules/ui_screen_gameover/helpers.hpp"

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

  if (game.running_state == GameState::START) {
    auto& io = ImGui::GetIO();

    const auto& window_pos = ImGui::GetWindowViewport()->GetCenter();
    const auto window_size_half = ImVec2(ImGui::GetWindowSize().x * 0.5f, ImGui::GetWindowSize().y * 0.5f);
    const auto pos = ImVec2(window_pos.x, window_pos.y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    bool show = true;
    ImGui::Begin("Welcome", &show, flags);

    ImGui::Text("Welcome!");
    ImGui::Text("");
    ImGui::Text("Move with the WASD keys. Collide with the enemies to damage them.");
    ImGui::Text("");
    ImGui::Text("¬¬ LEVELLING INFORMATION ¬¬");
    ImGui::Text("CON: +4 MAX HP");
    ImGui::Text("STR: +1 ATK");
    ImGui::Text("AGI: +1 DEF");
    ImGui::Text("");
    ImGui::Text("¬¬ Available Items ¬¬");
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