#include "system.hpp"

#include "components.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/sprites/helpers.hpp"
#include "helpers.hpp"
#include "modules/persistent/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/scene_helpers.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

#include <SDL2/SDL_log.h>
#include <format>
#include <string>

namespace game2d {
using namespace std::literals;

void
push_button_complete_colours()
{
  // Active state: Green
  ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 0.6f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.33f, 0.7f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.33f, 0.8f, 0.8f));
};

void
push_button_incomplete_colours()
{
  // Inactive state: red
  ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
};

void
update_ui_scene_main_menu(engine::SINGLE_Application& app, entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  auto& ui = get_first_component<SINGLE_MainMenuUI>(r);
  // const auto& input = get_first_component<SINGLE_InputComponent>(r);
  // const auto& controllers = input.controllers;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  // center
  // const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  // const auto& viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  // const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  // ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  // left third centered
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  const auto pos =
    ImVec2(viewport_pos.x + (ri.viewport_size_render_at.x * (3 / 12.0f)), viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("Main Menu", nullptr, flags);

  // TODO: keyboard to update ui? mouse could just click.
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

  const ImVec2 size = { 60, 20.0f };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, pivot);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));

  const auto selectable_button = [&](const std::string& label, int& selected, const int index) {
    // update the selected index if this button is clicked
    if (ImGui::Button(label.c_str(), size)) {
      do_ui_action = true;
      selected = index;
    }

    play_sound_if_hovered(r, ui.hovered_buttons, label);

    // Do the callback for the button
    if (selected == index && do_ui_action) {
      create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_SELECT_01" });
      return true;
    }

    return false;
  };

  {
    int index = 0;

    // std::filesystem probably wont work on web. figure this out.
    // if (std::filesystem::exists("save-overworld.json")) {
    //   if (selectable_button("Continue", selected, index++))
    //     move_to_scene_start(r, Scene::overworld, true);
    // }

    // if (selectable_button("Start", selected, index++))
    //   move_to_scene_additive(r, Scene::overworld);

    // if (selectable_button("SETTINGS", selected, index++)) {
    // }

#if defined(_DEBUG)
    push_button_incomplete_colours();
    if (selectable_button("Brawl!", selected, index++))
      move_to_scene_start(r, Scene::brawl);
    ImGui::PopStyleColor(3);
#endif

    // Exit button
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.6f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.6f, 0.8f, 0.8f));
    ImGui::NewLine();
    if (selectable_button("Exit", selected, index++))
      app.running = false;
    ImGui::PopStyleColor(3);
  }

  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  ImGui::End();

  // show a mute sound icon
  ui_mute_sound_icon(r);
};

} // namespace game2d