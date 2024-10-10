#include "system.hpp"

#include "engine/app/game_window.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/imgui/helpers.hpp"
#include "game_state.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/scene_helpers.hpp"

#include <SDL2/SDL_mixer.h>
#include <imgui.h>

#include <SDL2/SDL_log.h>
#include <algorithm>
#include <format>
#include <iterator>
#include <magic_enum.hpp>
#include <vector>

namespace game2d {

void
update_ui_pause_menu_system(engine::SINGLE_Application& app, entt::registry& r)
{
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiDockNodeFlags_NoResize;
  flags |= ImGuiDockNodeFlags_PassthruCentralNode;

  auto& input = get_first_component<SINGLE_InputComponent>(r);
  auto& state = get_first_component<SINGLE_GameStateComponent>(r);

  static bool open = false;

  // dont open pause menu in the main menu
  const auto& scene = get_first_component<SINGLE_CurrentScene>(r);
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

    ImGui::SetNextWindowSize({ 300, 400 });
    ImGui::Begin("Paused", &open, flags);

    ImGui::Text("Menu FPS: %0.2f", ImGui::GetIO().Framerate);

    const bool show_mousepos = false;
    if (show_mousepos) {
      const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
      ImGui::Text("imgui: %f, %f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
      const glm::ivec2 mouse_pos = get_mouse_pos() - ri.viewport_pos;
      ImGui::Text("mouse_pos: %i, %i", mouse_pos.x, mouse_pos.y);
      const glm::ivec2 worldspace_pos = mouse_position_in_worldspace(r);
      ImGui::Text("worldspace: %i, %i", worldspace_pos.x, worldspace_pos.y);
    }

    if (ImGui::Button("Resume"))
      open = false;

    ImGui::SeparatorText("Settings");

    static auto mode = engine::DisplayMode::windowed;
    static auto modes = engine::enum_class_to_vec_str<engine::DisplayMode>();
    ImGui::Text("Current mode: %s", convert_enum_to_string<engine::DisplayMode>(mode).c_str());

    WomboComboIn combo_in(modes);
    combo_in.label = "Display Mode";
    combo_in.current_index = static_cast<int>(mode);
    WomboComboOut combo_out = draw_wombo_combo(combo_in);
    if (combo_in.current_index != combo_out.selected) {
      mode = static_cast<engine::DisplayMode>(combo_out.selected);
      app.window.set_displaymode(mode);
    }

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

    static size_t idx = 0;
    WomboComboIn wc_in(resolutions_as_str);
    wc_in.label = "resolutions";
    wc_in.current_index = idx;
    const auto wc_out = draw_wombo_combo(wc_in);

    if (wc_out.selected != idx) {
      SDL_Log("%s", std::format("changing resolution").c_str());
      idx = wc_out.selected;
      app.window.set_size({ resolutions[idx].x, resolutions[idx].y });
    }

    const auto audio_e = get_first<SINGLE_AudioComponent>(r);
    if (audio_e != entt::null) {
      auto& audio = get_first_component<SINGLE_AudioComponent>(r);

      if (audio.loaded) {
        ImGui::SeparatorText("Audio");

        if (ImGui::SliderFloat("##max_volume", &audio.volume_user, 0.0f, 1.0f, "%.2f")) {
          audio.volume_internal = static_cast<int>(MIX_MAX_VOLUME * audio.volume_user);
          SDL_Log("%s", std::format("setting volume: {}", audio.volume_internal).c_str());
          for (int i = 0; i < audio.max_audio_sources; i++)
            Mix_Volume(i, audio.volume_internal);
        }

        static bool mute_all = audio.mute_all;
        if (ImGui::Checkbox("Mute All", &mute_all)) {
          audio.mute_all = mute_all;
          if (audio.mute_all)
            audio::sdl_mixer::stop_all_audio(r);
          else {
            // how to resume correct scene background music?
            // a better solution would be to fade the music back in
            // below is BAD.
            // const auto& s = get_first_component<SINGLE_CurrentScene>(r);
            // if (s.s == Scene::overworld)
            //   create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "GAME_01" });
            // if (s.s == Scene::dungeon_designer || s.s == Scene::turnbasedcombat) {
            // create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "COMBAT_01" });
          }
        }

        static bool mute_sfx = audio.mute_sfx;
        if (ImGui::Checkbox("Mute SFX", &mute_sfx))
          audio.mute_sfx = mute_sfx;
      }
    }

    ImGui::SeparatorText("Quit");

    // const auto& scene = get_first_component<SINGLE_CurrentScene>(r);
    // const bool is_saveable_scene = scene.s == Scene::overworld;
    // if (is_saveable_scene && ImGui::Button("Save to Menu")) {
    //   save(r, "save-overworld.json");
    //   move_to_scene_start(r, Scene::menu);
    // }

    if (ImGui::Button("Exit to Menu")) {
      move_to_scene_start(r, Scene::menu);

      open = false; // unpause this menu
    }

    if (ImGui::Button("Exit to Desktop"))
      app.running = false;

    ImGui::End();
  }
};

} // namespace game2d