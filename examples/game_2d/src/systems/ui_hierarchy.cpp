#include "systems/ui_hierarchy.hpp"

// components
#include "components/rendering.hpp"
#include "components/tag.hpp"

// other lib headers
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
  }
  ImGui::End();

  // update static var
  if (optional_eid != selected_entity)
    selected_entity = optional_eid;

  ImGui::Begin("Properties");
  if (selected_entity.value_or(entt::null) != entt::null) {

    const auto& eid = optional_eid.value_or(entt::null);

    if (registry.all_of<TagComponent>(eid)) {
      TagComponent& t = registry.get<TagComponent>(eid);

      // Able to change the value of the tag component
      char buffer[256];
      memset(buffer, 0, sizeof(buffer));
      strcpy_s(buffer, t.tag.c_str());

      ImGui::Text("Tag: ");
      ImGui::SameLine();
      if (ImGui::InputText(" ##tagbox", buffer, sizeof(buffer)))
        t.tag = std::string(buffer);
    }

    if (registry.all_of<PositionInt>(eid)) {
      const PositionInt& pi = registry.get<PositionInt>(eid);
      ImGui::Text("PositionInt: %i %i", pi.x, pi.y);
    }
  }
  ImGui::End();
};