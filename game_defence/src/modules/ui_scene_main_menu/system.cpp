#include "system.hpp"

#include "components.hpp"

#include "audio/components.hpp"
#include "audio/helpers/sdl_mixer.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "io/settings.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_inventory/helpers.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

#include <fmt/core.h>
#include <string>

namespace game2d {
using namespace std::literals;
using namespace audio::sdl_mixer;

void
update_ui_scene_main_menu(engine::SINGLETON_Application& app, entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto& controllers = input.controllers;
  const auto& ui = get_first_component<SINGLE_MainMenuUI>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  // center
  // const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  // const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  // const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  // ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  // left third centered
  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos =
    ImVec2(viewport_pos.x + (ri.viewport_size_current.x * (3 / 12.0f)), viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("Main Menu", nullptr, flags);

  // // TODO: keyboard to update ui? mouse could just click.
  // // Controller to update UI
  // if (controllers.size() > 0) {
  //   auto* c = controllers[0];
  //   if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X))
  //     selected++;
  //   if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y))
  //     selected--;
  //   if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A))
  //     do_ui_action = true;
  // }

  static int selected = 0;
  bool do_ui_action = false;
  // selected = selected < 0 ? buttons - 1 : selected;
  // selected %= buttons;

  const ImVec2 size = { 120, 120 * 9 / 16.0f };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, pivot);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));

  const auto selectable_button = [&](const std::string& label, int& selected, const int index) {
    // update the selected index if this button is clicked
    if (ImGui::Button(label.c_str(), size)) {
      do_ui_action = true;
      selected = index;
    }

    // Do the callback for the button
    if (selected == index && do_ui_action)
      return true;

    return false;
  };

  {
    int index = 0;

    // std::filesystem probably wont work on web. figure this out.
    // if (std::filesystem::exists("save-overworld.json")) {
    //   if (selectable_button("Continue", selected, index++))
    //     move_to_scene_start(r, Scene::overworld, true);
    // }

    if (selectable_button("START", selected, index++))
      move_to_scene_additive(r, Scene::overworld_revamped);

    if (selectable_button("SETTINGS", selected, index++)) {
    }

#if defined(_DEBUG)
    if (selectable_button("(minigame)\nbreach", selected, index++))
      move_to_scene_start(r, Scene::minigame_bamboo);

    if (selectable_button("(debug)\ncombat", selected, index++)) {
      move_to_scene_start(r, Scene::turnbasedcombat);
    }
    if (selectable_button("(debug)\ndungeon", selected, index++)) {
      move_to_scene_start(r, Scene::dungeon_designer);
      create_empty<RequestGenerateDungeonComponent>(r);
    }
#endif

    if (selectable_button("QUIT", selected, index++))
      app.running = false;
  }
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  ImGui::End();

  // show a sound icon
  ImGuiWindowFlags icon_flags = 0;
  icon_flags |= ImGuiWindowFlags_NoCollapse;
  icon_flags |= ImGuiWindowFlags_NoTitleBar;
  icon_flags |= ImGuiWindowFlags_AlwaysAutoResize;
  icon_flags |= ImGuiWindowFlags_NoBackground;

  const float distance_from_right_of_screen = 75;
  const float distance_from_top_of_screen = 75;

  const ImVec2 icon_size{ 50, 50 };
  static ImVec2 tl{ 0.0f, 0.0f };
  static ImVec2 br{ 1.0f, 1.0f };
  const auto set_icon_state = [&](const bool muted) {
    const auto [unmute_tl, unmute_br] = convert_sprite_to_uv(r, "AUDIO"s);
    const auto [mute_tl, mute_br] = convert_sprite_to_uv(r, "AUDIO_MUTE"s);
    if (muted) {
      tl = mute_tl;
      br = mute_br;
    } else {
      tl = unmute_tl;
      br = unmute_br;
    }
  };

  // button state
  static int mute = 0;

  // set state from saved disk
  static auto disk_preference_mute = gesert_string(PLAYERPREF_MUTE, "false"s) == "true";
  static bool set_from_disk = true;
  if (set_from_disk) {
    mute = disk_preference_mute;
    set_from_disk = false;
    set_icon_state(mute);
  }

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - distance_from_right_of_screen,
                                 viewport->WorkPos.y + viewport->WorkSize.y - distance_from_top_of_screen));
  ImGui::Begin("Mute Sound Icon", nullptr, icon_flags);

  // draw an audio icon
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "kennynl_gameicons")->id;
  const auto id = (ImTextureID)tex_id;

  bool toggle_changed = false;
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  if (ImGui::ImageButton("mute-icon", id, icon_size, tl, br)) {
    mute = mute == 1 ? 0 : 1;
    save_string(PLAYERPREF_MUTE, mute == 1 ? "true"s : "false"s);
    set_icon_state(mute);
    toggle_changed = true;
  }
  ImGui::PopStyleVar();

  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
  audio.mute_all = mute;
  audio.mute_sfx = mute;

  // toggle: mute to unmute. play menu theme.
  if (toggle_changed && mute == 0) {
    fmt::println("unmute all");
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "MENU_01" });
  }

  // toggle: unmute to mute. stop all music.
  if (toggle_changed && mute == 1) {
    fmt::println("muted all");
    stop_all_audio(r);
  }

  ImGui::End();
};

} // namespace game2d