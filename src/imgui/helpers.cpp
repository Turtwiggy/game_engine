#include "helpers.hpp"

// other lib
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

// std lib
#include <string>
#include <vector>

namespace game2d {

WomboComboOut
draw_wombo_combo(const WomboComboIn& in)
{
  const auto& label = in.label;
  const auto& items = in.items;
  const auto& cur_idx = in.current_index;

  if (items.size() == 0) {
    ImGui::Text("Empty WomboCombo: %s", in.label.c_str());
    WomboComboOut out;
    out.selected = 0;
    return out;
  }

  WomboComboOut out;
  out.selected = in.current_index;

  const std::string combo_preview_value = items[in.current_index];

  if (ImGui::BeginCombo(label.c_str(), combo_preview_value.c_str(), 0)) {
    for (size_t n = 0; n < items.size(); n++) {
      const bool is_selected = (cur_idx == n);

      if (ImGui::Selectable(items[n].c_str(), is_selected))
        out.selected = n;

      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  return out;
};

void
imgui_draw_string(const std::string& label, std::string& v)
{
  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  ImGui::InputText((std::string("##") + label).c_str(), &v);
};

void
imgui_draw_bool(const std::string& label, bool& v)
{
  bool v_temp = v;
  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::Checkbox((std::string("##") + label).c_str(), &v_temp))
    v = v_temp;
};

void
imgui_draw_int(const std::string& label, int& v)
{
  int v_temp = v;
  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::DragInt((std::string("##") + label).c_str(), &v_temp))
    v = v_temp;
};

void
imgui_draw_float(const std::string& label, float& v)
{
  float v_temp = v;

  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat((std::string("##") + label).c_str(), &v_temp, 0.01f))
    v = v_temp;
};

void
imgui_draw_ivec2(const std::string& label, glm::ivec2& xy)
{
  glm::ivec2 v_temp = xy;

  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::DragInt2((std::string("##") + label).c_str(), glm::value_ptr(v_temp), 0.5f)) {
    xy.x = v_temp.x;
    xy.y = v_temp.y;
  }
};

void
imgui_draw_ivec2(const std::string& label, int& x, int& y)
{
  glm::ivec2 v_temp = { x, y };

  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::DragInt2((std::string("##") + label).c_str(), glm::value_ptr(v_temp), 0.5f)) {
    x = v_temp.x;
    y = v_temp.y;
  }
};

void
imgui_draw_vec3(const std::string& label, glm::vec3& xyz, const float& v_speed)
{
  glm::vec3 v_temp = xyz;

  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat3((std::string("##") + label).c_str(), glm::value_ptr(v_temp), v_speed)) {
    xyz.x = v_temp.x;
    xyz.y = v_temp.y;
    xyz.z = v_temp.z;
  }
};

void
imgui_draw_vec3(const std::string& label, float& x, float& y, float& z, const float& v_speed)
{
  glm::vec3 v_temp = { x, y, z };

  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat3((std::string("##") + label).c_str(), glm::value_ptr(v_temp), v_speed)) {
    x = v_temp.x;
    y = v_temp.y;
    z = v_temp.z;
  }
};

void
imgui_draw_ivec3(const std::string& label, int& x, int& y, int& z)
{
  glm::ivec3 v_temp = { x, y, z };

  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::DragInt3((std::string("##") + label).c_str(), glm::value_ptr(v_temp), 0.5f)) {
    x = v_temp.x;
    y = v_temp.y;
    z = v_temp.z;
  }
};

void
imgui_draw_vec2(const std::string& label, glm::vec2& xy, const float v_speed)
{
  glm::vec2 v_temp = xy;

  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat2((std::string("##") + label).c_str(), glm::value_ptr(v_temp), v_speed)) {
    xy.x = v_temp.x;
    xy.y = v_temp.y;
  }
}

void
imgui_draw_vec2(const std::string& label, float& x, float& y, const float v_speed)
{
  glm::vec2 v_temp = { x, y };

  ImGui::Text("%s", label.c_str());
  ImGui::SameLine();
  if (ImGui::DragFloat2((std::string("##") + label).c_str(), glm::value_ptr(v_temp), v_speed)) {
    x = v_temp.x;
    y = v_temp.y;
  }
};

void
imgui_draw_entity(entt::registry& r,        //
                  const std::string& label, //
                  const entt::entity& e,    //
                  entt::entity& selected_e  //
)
{
  ImGuiTreeNodeFlags flags = ((selected_e == e) ? ImGuiTreeNodeFlags_Selected : 0);
  flags |= ImGuiTreeNodeFlags_OpenOnArrow;
  flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
  bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)e, flags, "%s", label.c_str());

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
};

} // namespace game2d