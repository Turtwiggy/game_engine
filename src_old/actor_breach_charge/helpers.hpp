#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
add_bomb_callback(entt::registry& r, const entt::entity e);

bool
debug_spawn_bombs(entt::registry& r, const glm::ivec2& mouse_pos_on_grid);

entt::entity
bomb_equipped_in_inventory(entt::registry& r);

} // namespace game2d