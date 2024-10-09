#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

std::vector<entt::entity>
contains_mobs(entt::registry& r, const glm::ivec2 gp);

} // namespace game2d