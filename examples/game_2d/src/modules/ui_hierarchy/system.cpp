#include "modules/ui_hierarchy/system.hpp"

// components
#include "modules/renderer/components.hpp"

// other lib headers
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

// c++ lib headers
#include <optional>

namespace game2d {
//
} // namespace game2d

void
game2d::init_ui_hierarchy_system(entt::registry& registry){
  //
};

void
game2d::update_ui_hierarchy_system(entt::registry& registry, engine::Application& app)
{
  // from a system no-data perspective, this is ILLEGAL
  static std::optional<entt::entity> selected_entity = std::nullopt;
  std::optional<entt::entity> optional_eid = selected_entity;

  ImGui::Begin("Hierarchy");
  {
    // List all entities...
    registry.each([&registry, &optional_eid](auto entity) {
      const auto& tag = registry.get<TagComponent>(entity).tag;
      const auto& eid = optional_eid.value_or(entt::null);

      ImGuiTreeNodeFlags flags = ((eid == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
      bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

      if (ImGui::IsItemClicked())
        optional_eid = entity;

      if (opened) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
        // bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
        // if (opened)
        //   ImGui::TreePop();
        // ImGui::Text("HERE?");
        ImGui::TreePop();
      }
    });

    // If select anywhere in the window, make entity unselected
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
      optional_eid = entt::null;
  }
  ImGui::End();

  // update static var
  if (optional_eid != selected_entity)
    selected_entity = optional_eid;

  ImGui::Begin("Properties");
  if (selected_entity.value_or(entt::null) != entt::null) {

    const auto& eid = optional_eid.value_or(entt::null);

    // Display TagComponent
    if (registry.all_of<TagComponent>(eid)) {
      TagComponent& t = registry.get<TagComponent>(eid);

      // Able to change the value of the tag component
      char buffer[256];
      memset(buffer, 0, sizeof(buffer));
      strcpy_s(buffer, t.tag.c_str());

      ImGui::Text("Tag: ");
      ImGui::SameLine();
      if (ImGui::InputText("##tagbox", buffer, sizeof(buffer)))
        t.tag = std::string(buffer);
    }

    // Display PositionIntComponent
    if (registry.all_of<PositionIntComponent>(eid)) {
      PositionIntComponent& pi = registry.get<PositionIntComponent>(eid);
      glm::ivec2 pos = { pi.x, pi.y };

      ImGui::Text("Pos: ");
      ImGui::SameLine();
      if (ImGui::DragInt2("##position", glm::value_ptr(pos), 0.5f)) {
        pi.x = pos.x;
        pi.y = pos.y;
      }
    }

    // Display SizeComponent
    if (registry.all_of<SizeComponent>(eid)) {
      SizeComponent& sc = registry.get<SizeComponent>(eid);

      // Able to change the value of PositionInt component
      glm::ivec2 size = { sc.w, sc.h };

      ImGui::Text("Size: ");
      ImGui::SameLine();
      if (ImGui::DragInt2("##size", glm::value_ptr(size), 0.5f)) {
        sc.w = size.x;
        sc.h = size.y;
      }
    }

    // Display ColourComponent
    if (registry.all_of<ColourComponent>(eid)) {
      ColourComponent& c = registry.get<ColourComponent>(eid);

      ImGui::Text("Colour: ");
      ImGui::SameLine();
      ImGui::ColorEdit4("##colour", glm::value_ptr(c.colour));
    }
  }
  ImGui::End();
};