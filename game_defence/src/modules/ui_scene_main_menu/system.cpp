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

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
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

  const auto selectable_button = [&do_ui_action](const std::string& label, int& selected, const int index) {
    const ImVec2 size = { 200, 50 };

    // ImGui::Text("selected: %i", selected);
    // ImGui::Text("Index: %i", index);

    // draw a button. highlight it if the selected index is over this button already
    ImGui::Selectable(label.c_str(), selected == index, 0, size);

    // update the selected index if this button is clicked
    if (ImGui::IsItemClicked()) {
      do_ui_action = true;
      selected = index;
    }

    // Do the callback for the button
    if (selected == index && do_ui_action)
      return true;

    return false;
  };

  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, pivot);
  {
    int index = 0;

    // std::filesystem probably wont work on web. figure this out.
    // if (std::filesystem::exists("save-overworld.json")) {
    //   if (selectable_button("Continue", selected, index++))
    //     move_to_scene_start(r, Scene::overworld, true);
    // }

    if (selectable_button("Start Game", selected, index++)) {
      move_to_scene_start(r, Scene::overworld, false);
    }

    if (selectable_button("minigame: bamboo", selected, index++)) {
      move_to_scene_start(r, Scene::minigame_bamboo);
    }

#if defined(_DEBUG)
    if (selectable_button("(debug) combat", selected, index++)) {
      move_to_scene_start(r, Scene::turnbasedcombat);
    }
    if (selectable_button("(debug) dungeon", selected, index++)) {
      move_to_scene_start(r, Scene::dungeon_designer);
      create_empty<RequestGenerateDungeonComponent>(r);
    }

#endif

    if (selectable_button("Quit", selected, index++))
      app.running = false;
  }
  ImGui::PopStyleVar();
  ImGui::End();

  // show a sound icon
  ImGuiWindowFlags icon_flags = 0;
  icon_flags |= ImGuiWindowFlags_NoCollapse;
  icon_flags |= ImGuiWindowFlags_NoTitleBar;
  icon_flags |= ImGuiWindowFlags_AlwaysAutoResize;
  icon_flags |= ImGuiWindowFlags_NoBackground;

  // window settings
  const int size_x = 1000;
  const int size_y = 500;
  const float cols_x = 20;
  const float cols_y = 10;
  const float pixels_x = size_x / cols_x;
  const float pixels_y = size_y / cols_y;
  const glm::ivec2 unmute_icon_offset{ 5, 0 };
  const glm::ivec2 mute_icon_offset{ 5, 1 };
  const ImVec2 icon_size = { 50, 50 };
  const float distance_from_right_of_screen = 75;
  const float distance_from_top_of_screen = 75;
  static glm::ivec2 offset = unmute_icon_offset;

  const auto set_icon_state = [&](const bool muted) {
    if (muted)
      offset = mute_icon_offset;
    else
      offset = unmute_icon_offset;
  };

  // button state
  static auto should_mute = gesert_string(PLAYERPREF_MUTE, "false"s) == "true";
  static int toggle = should_mute ? 1 : 0; // 1 is muted
  static bool toggle_changed = false;

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - distance_from_right_of_screen,
                                 viewport->WorkPos.y + viewport->WorkSize.y - distance_from_top_of_screen));
  ImGui::Begin("Mute Sound Icon", nullptr, icon_flags);

  // convert desired icon to uv coordinates
  // clang-format off
  ImVec2 tl = ImVec2(((offset.x * pixels_x + 0.0f    ) / size_x), ((offset.y * pixels_y + 0.0f    ) / size_y));
  ImVec2 br = ImVec2(((offset.x * pixels_x + pixels_x) / size_x), ((offset.y * pixels_y + pixels_y) / size_y));
  // clang-format on

  // draw an audio icon
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "kennynl_gameicons")->id;
  const auto id = (ImTextureID)tex_id;
  int frame_padding = -1;
  if (ImGui::ImageButton(id, icon_size, tl, br, frame_padding, ImColor(0, 0, 0, 255))) {
    toggle = toggle == 1 ? 0 : 1;
    toggle_changed = true;
    fmt::println("toggle changed to: {}", toggle);

    save_string(PLAYERPREF_MUTE, toggle == 1 ? "true"s : "false"s);
  }

  // update state
  set_icon_state(toggle);
  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
  audio.mute_all = toggle;
  audio.mute_sfx = toggle;

  // toggle: mute to unmute. play menu theme.
  if (toggle_changed && toggle == 0) {
    fmt::println("unmute all");
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "MENU_01" });
    toggle_changed = false;
  }

  // toggle: unmute to mute. stop all music.
  if (toggle_changed && toggle == 1) {
    fmt::println("muted all");
    stop_all_audio(r);
    toggle_changed = false;
  }

  ImGui::End();
};

} // namespace game2d