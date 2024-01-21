#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace game2d {

struct WomboComboIn
{
  std::string label;

  int current_index = 0;
  const std::vector<std::string>& items;

  WomboComboIn(const std::vector<std::string>& i)
    : items(i){};
};
struct WomboComboOut
{
  int selected = 0;
};

WomboComboOut
draw_wombo_combo(const WomboComboIn& in);

void
imgui_draw_entity(entt::registry& r, const std::string& label, const entt::entity& e, entt::entity& selected_e);

void
imgui_draw_string(const std::string& label, std::string& v);

void
imgui_draw_float(const std::string& label, float& v);

void
imgui_draw_ivec2(const std::string& label, int& x, int& y);

void
imgui_draw_vec3(const std::string& label, float& x, float& y, float& z);

void
imgui_draw_ivec3(const std::string& label, int& x, int& y, int& z);

void
imgui_draw_vec2(const std::string& label, float& x, float& y);

std::string
append_eid_to_label(const std::string& label, const entt::entity& e);

} // namespace game2d