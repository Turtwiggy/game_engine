#include "system.hpp"

#include "modules/events/helpers/keyboard.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_main_menu_system(engine::SINGLETON_Application& app, GameEditor& editor, Game& game)
{
  auto& input = game.input;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  static bool open = false;

  if (get_key_down(input, SDL_SCANCODE_ESCAPE))
    open = !open;

  // pause the game if the menu is open
  game.paused = open;

  if (open) {
    ImGui::Begin("Main Menu", &open, flags);

    if (ImGui::Button("Toggle Fullscreen"))
      app.window.toggle_fullscreen();
    ImGui::Text("Fullscreen currently: %i", app.window.get_fullscreen());

    if (ImGui::Button("Quit"))
      app.running = false;

    ImGui::End();
  }
};

} // namespace game2d