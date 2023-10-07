#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/controller.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_level_editor/components.hpp"
#include "modules/ui_level_editor/helpers.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {

void
update_ui_scene_main_menu(engine::SINGLETON_Application& app, entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto& controllers = input.controllers;
  auto& editor = get_first_component<SINGLETON_LevelEditor>(r);

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

  const ImVec2 size = { 100, 75 };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, pivot);

  // if (ImGui::Button("Instructions", size)) {
  //   // TODO
  // }
  if (ImGui::Button("Endless", size)) {
    editor.mode = LevelEditorMode::play;
    move_to_scene_start(r, Scene::game);
    // load(r, "assets/maps/main.json");
  }
  if (ImGui::Button("Map Edit", size)) {
    editor.mode = LevelEditorMode::edit;
    move_to_scene_start(r, Scene::game);
  }

  if (ImGui::Button("Quit", size))
    app.running = false;

  ImGui::PopStyleVar();

  ImGui::End();
};

} // namespace game2d