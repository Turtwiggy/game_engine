#pragma once

#include "modules/ui_hierarchy/components.hpp"

// other lib
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// std lib
#include <string>

namespace game2d {

entt::entity
create_hierarchy_root_node(entt::registry& r);

void
imgui_draw_string(entt::registry& r, const std::string& label, std::string& v);

void
imgui_draw_float(entt::registry& r, const std::string& label, float& v);

void
imgui_draw_ivec2(entt::registry& r, const std::string& label, int& x, int& y);

void
imgui_draw_vec2(entt::registry& r, const std::string& label, float& x, float& y);

void
imgui_draw_entity(entt::registry& r, const std::string& label, const entt::entity& e, entt::entity& selected_e);

void
drop_accept_entity(entt::registry& r, const entt::entity& e);

}; // namespace game2d