#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "game_state.hpp"
#include "imgui/helpers.hpp"
#include "modules/entt/serialize.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"

#include <imgui.h>

#include <algorithm>
#include <iterator>
#include <print>
#include <vector>

void
game2d::update_ui_pause_menu_system(engine::SINGLETON_Application& app, entt::registry& r)
{
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiDockNodeFlags_NoResize;
  flags |= ImGuiDockNodeFlags_PassthruCentralNode;

  auto& input = get_first_component<SINGLETON_InputComponent>(r);
  auto& state = get_first_component<SINGLETON_GameStateComponent>(r);

  static bool open = false;

  // dont open pause menu in the main menu
  const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  if (scene.s == Scene::menu) {
    open = false;
    return;
  }

  if (get_key_down(input, SDL_SCANCODE_ESCAPE)) {
    open = !open;
    ImGui::SetNextWindowFocus();
  }

  // only update state if it was running or paused
  if (open)
    state.state = state.state == GameState::RUNNING ? GameState::PAUSED : state.state;
  if (!open)
    state.state = state.state == GameState::PAUSED ? GameState::RUNNING : state.state;

  if (open) {
    // auto& io = ImGui::GetIO();
    // const auto& size = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    // ImGui::SetNextWindowPos(size, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    // const auto& viewport = ImGui::GetWindowViewport();
    // ImGui::SetNextWindowPos(viewport->GetCenter());
    // ImGui::SetNextWindowSize(ImVec2{ 300, 350 });

    ImGui::SetNextWindowSize({ 300, 300 });
    ImGui::Begin("Paused", &open, flags);

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

    static bool limit_fps = app.limit_fps;
    if (ImGui::Checkbox("Limit FPS", &limit_fps))
      app.limit_fps = limit_fps;

    static int i0 = 60;
    if (ImGui::InputInt("Target FPS", &i0))
      app.fps_limit = static_cast<float>(i0);

    struct Resolution
    {
      int x = 1920;
      int y = 1080;
    };

    static std::vector<Resolution> resolutions{
      { 1280, 720 },
      { 1600, 900 },
      { 1920, 1080 },
    };
    static std::vector<std::string> resolutions_as_str;
    static bool first_time = true;
    if (first_time) {

      // convert resolutions to string representation
      const auto convert_resolution_to_string = [](const Resolution& res) -> std::string {
        return { std::to_string(res.x) + "x" + std::to_string(res.y) };
      };
      std::transform(
        resolutions.begin(), resolutions.end(), std::back_inserter(resolutions_as_str), convert_resolution_to_string);

      first_time = false;
    }

    static WomboComboIn wc_in(resolutions_as_str);
    static WomboComboOut wc_out;
    wc_out = draw_wombo_combo(wc_in);

    if (wc_in.current_index != wc_out.selected) {
      std::print("changing resolution\n");
      const auto idx = wc_out.selected;
      app.window.set_size({ resolutions[idx].x, resolutions[idx].y });

      wc_in.current_index = wc_out.selected;
    }

    if (ImGui::Button("Back to Menu"))
      move_to_scene_start(r, Scene::menu);

    const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
    const bool is_saveable_scene = scene.s == Scene::overworld;
    if (is_saveable_scene && ImGui::Button("Save & Back to Menu")) {
      save(r, "save-overworld.json");
      move_to_scene_start(r, Scene::menu);
    }

    if (ImGui::Button("Close Application"))
      app.running = false;

    ImGui::End();
  }
};
