#pragma once

#include "modules/grid/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

GridComponent
map_to_grid(entt::registry& r);

void
update_path_to_tile_next_to_player(entt::registry& r, const entt::entity& src_e, const entt::entity& dst_e);

void
update_path_to_mouse(entt::registry& r, const entt::entity& src_e, const glm::ivec2& mouse_pos);

} // namespace game2d