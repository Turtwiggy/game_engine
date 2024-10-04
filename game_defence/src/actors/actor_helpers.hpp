#pragma once

#include "engine/colour/colour.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

[[nodiscard]] glm::vec2
get_position(entt::registry& r, const entt::entity e);

[[nodiscard]] glm::ivec2
get_grid_position(entt::registry& r, const entt::entity e);

void
set_position(entt::registry& r, const entt::entity e, const glm::vec2& pos);

void
set_position_grid(entt::registry& r, const entt::entity e, const glm::ivec2 grid_pos);

void
set_dir(entt::registry& r, const entt::entity e, const glm::vec2& dir);

[[nodiscard]] glm::vec2
get_size(entt::registry& r, const entt::entity e);

void
set_size(entt::registry& r, const entt::entity e, const glm::vec2& size);

void
set_colour(entt::registry& r, const entt::entity e, const engine::SRGBColour& col);

} // namespace game2d