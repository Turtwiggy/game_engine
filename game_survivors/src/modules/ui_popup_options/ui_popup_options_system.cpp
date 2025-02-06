#include "ui_popup_options_system.hpp"

#include "ui_popup_options_components.hpp"

#include "engine/app/game_window.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/core_camera/helpers.hpp"
#include "modules/effect_crt/crt_components.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"

#include <imgui.h>

namespace game2d {

// Press Start ->
// Show menu with resume,
// options,
// quit to menu (are you sure prompt),
// quit to desktop (are you sure prompt)
// Options menu... (reusable from the menu screen)
// Video
// ImGui::Text("Window");
// ImGui::Text("Monitor");
// ImGui::Text("Resolution");
// ImGui::Text("VSync")
// ImGui::Text("Limit Framerate");
// appear if the above is yes,
// ImGui::Text("Limit");
// Audio
// sliders...
// Effects
// toggles...
// Back to pause menu

// ui elements needed...
// fixed options: text, ab left, text, ab right (resolution)
// toggles: on/off (effects)
// sliders (audio)

void
update_ui_popup_options_system(entt::registry& r)
{
  static bool open = false;

  process_requests<RequestToShowOptionsMenu>(r, []() { open = true; });
  if (!open)
    return;

  ImGui::Begin("Options Menu");

  ImGui::Text("Temporary");

  /*

  ImGui::SeparatorText("Settings");

  static auto mode = engine::DisplayMode::windowed;
  static auto modes = engine::enum_class_to_vec_str<engine::DisplayMode>();
  // ImGui::Text("Current mode: %s", convert_enum_to_string<engine::DisplayMode>(mode).c_str());

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

  std::string separator_label = std::format("Screen Size ({}, {})", ri.viewport_size_current.x,
  ri.viewport_size_current.y); ImGui::SeparatorText(separator_label.c_str());

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

  */

  ImGui::End();
}

} // namespace game2d