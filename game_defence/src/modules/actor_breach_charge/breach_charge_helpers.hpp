#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {

void
add_bomb_callback(entt::registry& r, const entt::entity e);

bool
debug_spawn_bombs(entt::registry& r, const glm::ivec2& mouse_pos_on_grid);

// return the bomb item or entt::null
entt::entity
bomb_equipped_in_inventory(entt::registry& r);

void
DrawZeldaCursor(const ImVec2& pos, const ImVec2& window_size, float radius, float thickness, float angle, ImU32 color);

void
DrawZeldaCursorWindow(entt::registry& r, const glm::ivec2& mouse_pos, const float angle);

} // namespace game2d