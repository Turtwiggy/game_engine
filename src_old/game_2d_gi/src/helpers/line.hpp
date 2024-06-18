#pragma once

#include "modules/renderer/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

// #include <utility>
// #include <vector>

namespace game2d {

struct LineInfo
{
  glm::vec2 position{ 0, 0 };
  float rotation = 0.0f;
  glm::vec2 scale;
};

LineInfo
generate_line(const glm::ivec2& a, const glm::ivec2& b, int width);

// inspired by:
// https://github.com/scikit-image/scikit-image/blob/main/skimage/draw/_draw.pyx
// void
// create_line(int r0, int c0, int r1, int c1, std::vector<std::pair<int, int>>& results);

} // game2d