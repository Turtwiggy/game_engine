#pragma once

#include "modules/grid/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
add_entity_to_map(entt::registry& r, const entt::entity src_e, const int idx);

bool
move_entity_on_map(entt::registry& r, const int idx_a, const int idx_b);

std::vector<glm::ivec2>
generate_path(entt::registry& r, const entt::entity src_e, const glm::ivec2& worldspace_pos, const size_t limit);

} // namespace game2d