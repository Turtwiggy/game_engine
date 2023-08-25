#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/controller.hpp"
#include "modules/scene/helpers.hpp"
#include "renderer/components.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {

void
update_ui_scene_main_menu(engine::SINGLETON_Application& app, entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto& controllers = input.controllers;

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

  static int selected = 0;
  bool do_ui_action = false;

  // TODO: keyboard to update ui? mouse could just click.
  // Controller to update UI
  if (controllers.size() > 0) {
    auto* c = controllers[0];
    if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X))
      selected++;
    if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y))
      selected--;
    if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A))
      do_ui_action = true;
  }

  const int buttons = 2;
  selected = selected < 0 ? buttons - 1 : selected;
  selected %= buttons;

  const ImVec2 size = { 100, 75 };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, pivot);

  ImGui::Selectable("Play", selected == 0, 0, size);
  if (ImGui::IsItemClicked()) {
    do_ui_action = true;
    selected = 0;
  }
  if (selected == 0 && do_ui_action)
    move_to_scene_start(r, Scene::game);

  ImGui::Selectable("Quit", selected == 1, 0, size);
  if (ImGui::IsItemClicked()) {
    do_ui_action = true;
    selected = 1;
  }
  if (selected == 1 && do_ui_action)
    app.running = false;

  ImGui::PopStyleVar();

  //   if (focused_element == 0)
  //     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 0, 1));
  //   if (ImGui::Button("Play".c_str(), button_size))
  //   if (focused_element == 0)
  //     ImGui::PopStyleColor();

  //   if (focused_element == 1)
  //     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 0, 1));
  //   if (ImGui::Button("Quit", button_size))
  //     app.running = false;
  //   if (focused_element == 1)
  //     ImGui::PopStyleColor();

  ImGui::End();
};

} // namespace game2d