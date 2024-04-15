#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

glm::vec2
get_position(entt::registry& r, const entt::entity& e);

void
set_position(entt::registry& r, const entt::entity& e, const glm::ivec2& pos);

void
set_size(entt::registry& r, const entt::entity& e, const float& size);
void
set_size(entt::registry& r, const entt::entity& e, const glm::ivec2& size);

} // namespace game2d