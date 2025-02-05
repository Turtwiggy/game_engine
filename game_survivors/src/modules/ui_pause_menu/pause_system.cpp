#include "pause_system.hpp"

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
#include "modules/effect_crt/crt_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mixer.h>
#include <imgui.h>
#include <magic_enum.hpp>

#include <algorithm>
#include <format>
#include <iterator>
#include <vector>

namespace game2d {

void
update_ui_pause_menu_system(engine::SINGLE_Application& app, entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input)
  auto& state = get_first_component<SINGLE_GameStateComponent>(r);

  static bool open = false;

  // dont open pause menu in the main menu
  // const auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  // if (scene.s == Scene::menu) {
  //   open = false;
  //   return;
  // }

  // Issue:
  // if you hold a button and that buttons swaps the action set,
  // the button-down in the new action set will be true.
  // Sol(?):
  // Now only use the "game" action set for the survive scene,
  // even when a menu is open.

  for (int i = 0; i < steam_c.n_active; i++) {
    const auto handle = steam_c.handles[i];
    const auto game_pause = controller_button_down(steam_c, handle, DA::Game_Pause);

    if (game_pause) {
      open = !open;
      break;
    }
  }

  //
  // Keyboard Support.
  //

  if (get_key_down(input, SDL_SCANCODE_ESCAPE)) {
    open = !open;
    ImGui::SetNextWindowFocus();
  }

  // only update state if it was running or paused
  if (open)
    state.state = state.state == GameState::RUNNING ? GameState::PAUSED : state.state;
  if (!open)
    state.state = state.state == GameState::PAUSED ? GameState::RUNNING : state.state;
  if (!open)
    return;

  const auto half_wh = ImVec2{ ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f };
  const auto center = ImVec2{ (float)ri.viewport_pos.x + half_wh.x, (float)ri.viewport_pos.y + half_wh.y };
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, { 0.5f, 0.5f });

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiDockNodeFlags_PassthruCentralNode;

  // ImGui::SetNextWindowSize({ 300, 600 });
  ImGui::Begin("Paused", NULL, flags);

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
    // change display mode
    mode = static_cast<engine::DisplayMode>(combo_out.selected);
    app.window.set_displaymode(mode);

    // center the newly windowed window
    if (mode == engine::DisplayMode::windowed || mode == engine::DisplayMode::windowed_borderless) {
      SDL_DisplayMode DM;
      SDL_GetCurrentDisplayMode(0, &DM);
      const glm::ivec2 display_size = { DM.w, DM.h };
      const glm::ivec2 window_size = app.window.get_size();
      const auto pos = (display_size / 2) - (window_size / 2);
      app.window.set_position(pos.x, pos.y);
    }
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

  std::string separator_label = std::format("Screen Size ({}, {})", ri.viewport_size_current.x, ri.viewport_size_current.y);
  ImGui::SeparatorText(separator_label.c_str());

  struct Resolution
  {
    int x = 1920;
    int y = 1080;
  };

  static std::vector<Resolution> resolutions{
    { 1280, 720 },
    { 1920, 1080 },
  };

  static std::vector<std::string> resolutions_as_str;
  static bool first_time = true;
  if (first_time) {

    const auto convert_resolution_to_string = [](const Resolution& res) -> std::string {
      return { std::to_string(res.x) + "x" + std::to_string(res.y) };
    };

    // convert resolutions to string representation
    std::transform(
      resolutions.begin(), resolutions.end(), std::back_inserter(resolutions_as_str), convert_resolution_to_string);

    first_time = false;
  }

  auto align_right_button = [](std::string label) -> bool {
    // Align the button to the right
    const float available_width = ImGui::GetContentRegionAvail().x;
    const float button_width = 80.0f; // Set the desired button width
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available_width - button_width);
    return ImGui::Button(label.c_str(), ImVec2(button_width, 0));
  };

  for (size_t i = 0; i < resolutions.size(); i++) {
    ImGui::Text("%s", resolutions_as_str[i].c_str());
    ImGui::SameLine();

    std::string label = "Apply##" + std::to_string(i);
    if (align_right_button(label.c_str())) {
      app.window.set_size({ resolutions[i].x, resolutions[i].y });
      SDL_Log("changing resolution");
    }
  }

  static int custom_x = 1920;
  static int custom_y = 1080;

  ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 5.0f);
  {
    imgui_draw_int("x", custom_x);
    ImGui::SameLine();
    imgui_draw_int("y", custom_y);
    custom_x = glm::max(custom_x, 400);
    custom_y = glm::max(custom_y, 400);

    ImGui::SameLine();
    if (align_right_button("Apply##custom"))
      app.window.set_size({ custom_x, custom_y });
  }
  ImGui::PopItemWidth();

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

  ImGui::SeparatorText("Effects");

  auto& crt_c = get_first_component<SINGLE_EffectCrt>(r);
  ImGui::Checkbox("CRT", &crt_c.enabled);

  static bool grid_effect = true;
  ImGui::Checkbox("Grid", &grid_effect);
  const auto grid_e = get_first<Effect_GridComponent>(r);
  if (grid_effect == true && grid_e == entt::null)
    create_empty<Effect_GridComponent>(r);
  if (grid_effect == false && grid_e != entt::null)
    r.destroy(grid_e);

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
};

} // namespace game2d