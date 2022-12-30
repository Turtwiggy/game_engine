#include "system.hpp"

#include "events/helpers/keyboard.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_main_menu_system(engine::SINGLETON_Application& app, GameEditor& editor, Game& game)
{
  auto& input = game.input;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiDockNodeFlags_PassthruCentralNode;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiDockNodeFlags_NoResize;

  static bool open = false;

  if (get_key_down(input, SDL_SCANCODE_ESCAPE))
    open = !open;

  // only update state if it was running or paused
  if (game.running_state == GameState::RUNNING || game.running_state == GameState::PAUSED)
    game.running_state = open ? GameState::PAUSED : GameState::RUNNING;

  if (open) {
    // auto& io = ImGui::GetIO();
    // const auto& size = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    // ImGui::SetNextWindowPos(size, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    // const auto& viewport = ImGui::GetWindowViewport();
    // ImGui::SetNextWindowPos(viewport->GetCenter());
    ImGui::SetNextWindowSize(ImVec2{ 300, 350 });

    ImGui::Begin("Main Menu", &open, flags);

    ImGui::Text("Menu FPS: %0.2f", ImGui::GetIO().Framerate);

    if (ImGui::Button("Resume"))
      open = false;

    static bool value = false;
    if (ImGui::Checkbox("Fullscreen", &value))
      app.window.toggle_fullscreen();

    static bool borderless = false;
    if (ImGui::Checkbox("Borderless", &borderless))
      app.window.set_bordered(!borderless);

    static bool vsync = app.vsync;
    if (ImGui::Checkbox("VSync", &vsync))
      app.window.set_vsync_opengl(vsync);

    static bool limit_fps = app.vsync;
    if (ImGui::Checkbox("Limit FPS", &limit_fps))
      app.limit_fps = limit_fps;

    static int i0 = 60;
    if (ImGui::InputInt("Target FPS", &i0))
      app.fps_limit = static_cast<float>(i0);

    const char* items[] = { "1600x900", "1920x1080", "2560x1440", "3840x2160" };
    static int item_current = 0;
    if (ImGui::Combo("##resolutions", &item_current, items, IM_ARRAYSIZE(items))) {
      if (item_current == 0)
        app.window.set_size({ 1600, 900 });
      if (item_current == 1)
        app.window.set_size({ 1920, 1080 });
      if (item_current == 2)
        app.window.set_size({ 2560, 1440 });
      if (item_current == 3)
        app.window.set_size({ 3840, 2160 });
    }

    // if (ImGui::Button("Restart"))
    //   game.running_state = GameState::START;

    if (ImGui::Button("Quit"))
      app.running = false;

    ImGui::End();
  }
};

} // namespace game2d