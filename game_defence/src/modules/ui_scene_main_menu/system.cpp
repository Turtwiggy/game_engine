#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/scene/helpers.hpp"
#include "renderer/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_scene_main_menu(engine::SINGLETON_Application& app, entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  // bug: static bool doenst reset when return to menu...
  static bool first_time = true;
  if (first_time) {
    ImGui::SetNextWindowFocus();
  }

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("Main Menu", nullptr, flags);

  std::string label = "Play";

  if (first_time) {
    ImGui::SetKeyboardFocusHere(0);
  }

  ImVec2 button_size = { 100, 75 };
  if (ImGui::Button(label.c_str(), button_size))
    move_to_scene_start(r, Scene::game);

  // if (ImGui::Button("Settings", button_size)) {
  //   // do something else
  // }

  if (ImGui::Button("Quit", button_size)) {
    app.running = false;
  }

  ImGui::End();

  first_time = false;
};

} // namespace game2d