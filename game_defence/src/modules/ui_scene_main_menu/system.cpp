#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/scene/helpers.hpp"
#include "renderer/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_scene_main_menu(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiDockNodeFlags_NoResize;
  flags |= ImGuiDockNodeFlags_PassthruCentralNode;

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  // const auto& viewport = ImGui::GetWindowViewport();
  // ImGui::SetNextWindowPos(viewport->GetCenter());

  ImGui::Begin("Main Menu", nullptr, flags);

  std::string label = "Play";

  {
    // ImGuiStyle& style = ImGui::GetStyle();
    // float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
    // float avail = ImGui::GetContentRegionAvail().x;
    // float off = (avail - size) * alignment;
    // if (off > 0.0f)
    //   ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
  }

  static ImVec2 button_size = { 100, 100 };
  if (ImGui::Button(label.c_str(), button_size))
    move_to_scene_start(r, Scene::game);

  ImGui::End();
};

} // namespace game2d