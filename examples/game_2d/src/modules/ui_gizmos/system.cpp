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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void
game2d::init_ui_gizmos_system(entt::registry& registry)
{
  registry.set<SINGLETON_GizmosComponent>(SINGLETON_GizmosComponent());
}

void
game2d::update_ui_gizmos_system(entt::registry& registry, engine::Application& app, float dt)
{
  auto& gizmos_component = registry.ctx<SINGLETON_GizmosComponent>();
  const auto& hierarchy_component = registry.ctx<SINGLETON_HierarchyComponent>();
  const auto& r = registry.ctx<SINGLETON_RendererInfo>();

  ImGuizmo::BeginFrame();

  if (app.get_input().get_key_down(SDL_SCANCODE_Q))
    gizmos_component.gizmo_type = -1;
  if (app.get_input().get_key_down(SDL_SCANCODE_W))
    gizmos_component.gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
  if (app.get_input().get_key_down(SDL_SCANCODE_E))
    gizmos_component.gizmo_type = ImGuizmo::OPERATION::ROTATE;
  if (app.get_input().get_key_down(SDL_SCANCODE_R))
    gizmos_component.gizmo_type = ImGuizmo::OPERATION::SCALE;

  ImGui::Begin("ImGuizmo Debug");
  ImGui::Text("Type: %i", gizmos_component.gizmo_type);

  if (hierarchy_component.selected_entity != entt::null && gizmos_component.gizmo_type != -1) {
    ImGui::Text("Something selected");

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::SetDrawlist();

    float window_width = (float)r.viewport_size_current.x;
    float window_height = (float)r.viewport_size_current.y;
    ImGuizmo::SetRect(r.viewport_pos.x, r.viewport_pos.y, window_width, window_height);

    auto& camera_projection = r.projection;
    glm::mat4 camera_view = glm::inverse(glm::mat4(1.0f));
    camera_view = glm::translate(camera_view, glm::vec3(0.0f, 0.0f, 0.0f));
    // model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    // model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    // model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    // model = glm::scale(model, glm::vec3(size, 1.0f));

    glm::mat4 transform = glm::mat4(1.0f);
    if (registry.all_of<PositionIntComponent>(hierarchy_component.selected_entity)) {
      PositionIntComponent& pic = registry.get<PositionIntComponent>(hierarchy_component.selected_entity);
      transform = glm::translate(transform, glm::vec3(pic.x, pic.y, 0.0f));
      // RenderSizeComponent& rsc = registry.get<RenderSizeComponent>(eid);
      // ColourComponent& cc = registry.get<ColourComponent>(eid);
    };

    //
    // ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), matrixTranslation, matrixRotation, matrixScale);
    // ImGui::DragFloat3("Tr", matrixTranslation);
    // ImGui::DragFloat3("Rt", matrixRotation);
    // ImGui::DragFloat3("Sc", matrixScale);
    // ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale,
    // glm::value_ptr(transform));

    ImGuizmo::Manipulate(glm::value_ptr(camera_view),
                         glm::value_ptr(camera_projection),
                         (ImGuizmo::OPERATION)gizmos_component.gizmo_type,
                         ImGuizmo::LOCAL,
                         glm::value_ptr(transform),
                         nullptr,
                         nullptr);

    if (ImGuizmo::IsUsing()) {
      ImGui::Text("Using gizmo");

      // check new transform values
      // float matrixTranslation[3], matrixRotation[3], matrixScale[3];
      // ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), matrixTranslation, matrixRotation,
      // matrixScale);

      // todo: update entity transform...
    };
  }

  ImGui::End();
};