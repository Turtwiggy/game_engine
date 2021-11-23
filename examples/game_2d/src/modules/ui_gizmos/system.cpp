#include "modules/ui_gizmos/system.hpp"

// components
#include "modules/ui_gizmos/components.hpp"
#include "modules/ui_hierarchy/components.hpp"

// other lib headers
// clang-format off
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
// clang-format on

void
game2d::init_ui_gizmos_system(entt::registry& registry)
{
  registry.set<SINGLETON_GizmosComponent>(SINGLETON_GizmosComponent());
}

void
game2d::update_ui_gizmos_system(entt::registry& registry, engine::Application& app, float dt){
  // auto& gizmos_component = registry.ctx<SINGLETON_GizmosComponent>();
  // if (app.get_input().get_key_down(SDL_SCANCODE_Q))
  //   gizmos_component.gizmo_type = -1;
  // if (app.get_input().get_key_down(SDL_SCANCODE_W))
  //   gizmos_component.gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
  // if (app.get_input().get_key_down(SDL_SCANCODE_E))
  //   gizmos_component.gizmo_type = ImGuizmo::OPERATION::ROTATE;
  // if (app.get_input().get_key_down(SDL_SCANCODE_R))
  //   gizmos_component.gizmo_type = ImGuizmo::OPERATION::SCALE;

  // ImGui::Begin("ImGuizmo Debug");
  // ImGui::Text("Type: %i", gizmos_component.gizmo_type);

  // const auto& hierarchy_component = registry.ctx<SINGLETON_HierarchyComponent>();
  // if (hierarchy_component.selected_entity != entt::null) {
  //   ImGui::Text("Something selected");

  //   // Guizmos
  //   // ImGuizmo::SetOrthographic(false);
  //   // ImGuizmo::SetDrawlist();
  //   // float window_width = (float)ImGui::GetWindowWidth();
  //   // float window_height = (float)ImGui::GetWindowHeight();
  //   // ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGu::GetWindowPos().y, window_width, window_height);
  // }

  // ImGui::End();
};