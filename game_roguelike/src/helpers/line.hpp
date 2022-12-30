#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace game2d {

void
set_line(entt::registry& r, const entt::entity& line, const glm::ivec2& a, const glm::ivec2& b);

// inspired by:
// https://github.com/scikit-image/scikit-image/blob/main/skimage/draw/_draw.pyx
void
create_line(int r0, int c0, int r1, int c1, std::vector<std::pair<int, int>>& results);

} // game2d