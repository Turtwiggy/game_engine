#pragma once

// other lib
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// std lib
#include <string>

namespace game2d {

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

}; // namespace game2d