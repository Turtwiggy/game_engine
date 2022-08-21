#include "helpers.hpp"

// my lib
#include "modules/renderer/components.hpp"
#include "modules/ui_hierarchy/components.hpp"

// other lib
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

// std lib
#include <algorithm>
#include <stdio.h>
#include <vector>

entt::entity
game2d::create_hierarchy_root_node(entt::registry& r)
{
  auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
  h.root_node = r.create();

  r.emplace<TagComponent>(h.root_node, "root-node");
  r.emplace<EntityHierarchyComponent>(h.root_node, h.root_node);

  return h.root_node;
};

void
game2d::imgui_draw_string(entt::registry& r, const std::string& label, std::string& v)
{
  ImGui::Text(label.c_str());
  ImGui::SameLine();
  ImGui::InputText((std::string("##") + label).c_str(), &v);
}

void
game2d::imgui_draw_float(entt::registry& r, const std::string& label, float& v)
{
  float v_temp = v;

  ImGui::Text(label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat((std::string("##") + label).c_str(), &v_temp, 0.5f))
    v = v_temp;
}

void
game2d::imgui_draw_ivec2(entt::registry& r, const std::string& label, int& x, int& y)
{
  glm::ivec2 v_temp = { x, y };

  ImGui::Text(label.c_str());
  ImGui::SameLine();
  if (ImGui::DragInt2((std::string("##") + label).c_str(), glm::value_ptr(v_temp), 0.5f)) {
    x = v_temp.x;
    y = v_temp.y;
  }
}

void
game2d::imgui_draw_vec2(entt::registry& r, const std::string& label, float& x, float& y)
{
  glm::vec2 v_temp = { x, y };

  ImGui::Text(label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat2((std::string("##") + label).c_str(), glm::value_ptr(v_temp), 0.5f)) {
    x = v_temp.x;
    y = v_temp.y;
  }
}

void
game2d::imgui_draw_entity(entt::registry& r, const std::string& label, const entt::entity& e, entt::entity& selected_e)
{
  ImGuiTreeNodeFlags flags = ((selected_e == e) ? ImGuiTreeNodeFlags_Selected : 0);
  flags |= ImGuiTreeNodeFlags_OpenOnArrow;
  flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
  bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)e, flags, label.c_str());

  if (ImGui::IsItemClicked())
    selected_e = e;

  // Right click on the dropdown entry to delete it
  bool delete_entity = false;
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete Entity")) {
      delete_entity = true;
    }
    ImGui::EndPopup();
  }

  if (ImGui::BeginDragDropSource()) {
    ImGui::SetDragDropPayload("DEMO", &selected_e, sizeof(entt::entity));
    ImGui::Text(label.c_str());
    ImGui::EndDragDropSource();
  }

  drop_accept_entity(r, e);

  if (delete_entity)
    printf("todo: implement delete");

  if (opened) {

    const auto& h = r.get<EntityHierarchyComponent>(e);
    for (const auto& child : h.children) {
      const auto& new_tag = r.get<TagComponent>(child).tag;
      imgui_draw_entity(r, new_tag, child, selected_e);
    }

    ImGui::TreePop();
  }
}

void
game2d::drop_accept_entity(entt::registry& r, const entt::entity& e)
{
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DEMO")) {
      IM_ASSERT(payload->DataSize == sizeof(entt::entity));
      entt::entity payload_n = *(const entt::entity*)payload->Data;
      // std::cout << "received payload: " << static_cast<uint32_t>(payload_n) << std::endl;

      if (payload_n != e) { // payload entity isn't this entity

        // iterate over all entity-hierarchy-components
        // if any of them contained the payload n as a child, remove it
        auto view = r.view<EntityHierarchyComponent>();
        view.each([&payload_n](EntityHierarchyComponent& hierarchy) {
          hierarchy.children.erase(std::remove(hierarchy.children.begin(), hierarchy.children.end(), payload_n),
                                   hierarchy.children.end());
        });

        // add payload to this entity's hierarchy
        auto& current_h = r.get<EntityHierarchyComponent>(e);
        current_h.children.push_back(payload_n); // add as child

        // set the parent for the payload entity's hierarchy
        auto& old_h = r.get<EntityHierarchyComponent>(payload_n);
        old_h.parent = e; // set new parent
      }
    }
    ImGui::EndDragDropTarget();
  }
}
