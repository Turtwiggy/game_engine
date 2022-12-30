#include "helpers.hpp"

// my lib
#include "components/actors.hpp"
#include "components/app.hpp"
#include "lifecycle/components.hpp"
#include "renderer/components.hpp"
#include "ui_hierarchy/components.hpp"

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
  auto h = r.create();

  r.emplace<TagComponent>(h, std::string("EMPTY"));
  r.emplace<EntityTypeComponent>(h, EntityType::empty);
  r.emplace<EntityHierarchyComponent>(h);
  r.emplace<RootNode>(h);
  return h;
};

void
game2d::imgui_draw_string(const std::string& label, std::string& v)
{
  ImGui::Text(label.c_str());
  ImGui::SameLine();
  ImGui::InputText((std::string("##") + label).c_str(), &v);
};

void
game2d::imgui_draw_float(const std::string& label, float& v)
{
  float v_temp = v;

  ImGui::Text(label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat((std::string("##") + label).c_str(), &v_temp, 0.5f))
    v = v_temp;
};

void
game2d::imgui_draw_ivec2(const std::string& label, int& x, int& y)
{
  glm::ivec2 v_temp = { x, y };

  ImGui::Text(label.c_str());
  ImGui::SameLine();
  if (ImGui::DragInt2((std::string("##") + label).c_str(), glm::value_ptr(v_temp), 0.5f)) {
    x = v_temp.x;
    y = v_temp.y;
  }
};

void
game2d::imgui_draw_vec3(const std::string& label, float& x, float& y, float& z)
{
  glm::vec3 v_temp = { x, y, z };

  ImGui::Text(label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat3((std::string("##") + label).c_str(), glm::value_ptr(v_temp), 0.5f)) {
    x = v_temp.x;
    y = v_temp.y;
    z = v_temp.z;
  }
};

void
game2d::imgui_draw_ivec3(const std::string& label, int& x, int& y, int& z)
{
  glm::ivec3 v_temp = { x, y, z };

  ImGui::Text(label.c_str());
  ImGui::SameLine();
  if (ImGui::DragInt3((std::string("##") + label).c_str(), glm::value_ptr(v_temp), 0.5f)) {
    x = v_temp.x;
    y = v_temp.y;
    z = v_temp.z;
  }
};

void
game2d::imgui_draw_vec2(const std::string& label, float& x, float& y)
{
  glm::vec2 v_temp = { x, y };

  ImGui::Text(label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat2((std::string("##") + label).c_str(), glm::value_ptr(v_temp), 0.5f)) {
    x = v_temp.x;
    y = v_temp.y;
  }
};

namespace game2d {

void
recursively_delete_with_children(GameEditor& editor, Game& game, const entt::entity& e)
{
  auto& r = game.state;
  auto& eb = game.dead;
  const auto& h = r.get<EntityHierarchyComponent>(e);

  // delete children first. haha.
  for (auto& child : h.children)
    recursively_delete_with_children(editor, game, child);

  // remove myself from parent
  auto* parent_h = r.try_get<EntityHierarchyComponent>(h.parent);
  for (int i = 0; const auto& child : parent_h->children) {
    if (child == e)
      parent_h->children.erase(parent_h->children.begin() + i);
    i++;
  }

  eb.dead.emplace(e);
};

} // namespace game2d

void
game2d::imgui_draw_entity(GameEditor& editor,
                          Game& game,
                          const std::string& label,
                          const entt::entity& e,
                          entt::entity& selected_e)
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

  auto& r = game.state;

  drop_accept_entity(r, e);

  if (opened) {
    const auto& h = r.get<EntityHierarchyComponent>(e);
    for (const auto& child : h.children) {
      const auto& new_tag = r.get<TagComponent>(child).tag;
      imgui_draw_entity(editor, game, new_tag, child, selected_e);
    }

    ImGui::TreePop();
  }

  if (delete_entity)
    recursively_delete_with_children(editor, game, e);
};

void
game2d::drop_accept_entity(entt::registry& r, const entt::entity& e)
{
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DEMO")) {
      IM_ASSERT(payload->DataSize == sizeof(entt::entity));
      entt::entity payload_n = *(const entt::entity*)payload->Data;
      // std::cout << "received payload: " << static_cast<uint32_t>(payload_n) << "\n";

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
};
