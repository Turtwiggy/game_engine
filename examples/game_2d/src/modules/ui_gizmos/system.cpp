#include "modules/ui_gizmos/system.hpp"

// components
#include "modules/renderer/components.hpp"
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
  // const auto& hierarchy_component = registry.ctx<SINGLETON_HierarchyComponent>();
  // const auto& r = registry.ctx<SINGLETON_RendererInfo>();

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

  // if (hierarchy_component.selected_entity != entt::null && gizmos_component.gizmo_type != -1) {
  //   ImGui::Text("Something selected");

  //   ImGuizmo::SetOrthographic(false);
  //   ImGuizmo::SetDrawlist();

  //   float window_width = (float)r.viewport_size_current.x;
  //   float window_height = (float)r.viewport_size_current.y;
  //   ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, window_width, window_height);

  //   ImGuizmo::Manipulate()
  // }

  // ImGui::End();
};