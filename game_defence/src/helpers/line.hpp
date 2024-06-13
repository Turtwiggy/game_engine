#pragma once

#include "modules/renderer/components.hpp"
#include "renderer/transform.hpp"

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

struct Line
{
  glm::ivec2 p0{ 0, 0 };
  glm::ivec2 p1{ 0, 0 };

  int room_debug_dir = -1;
  int room_debug_idx = -1;
};
inline bool
operator==(const Line& a, const Line& b)
{
  return a.p0 == b.p0 && b.p1 == b.p1;
};

LineInfo
generate_line(const glm::ivec2& a, const glm::ivec2& b, int width);

void
set_transform_with_line(TransformComponent& t, const LineInfo& li);

void
set_transform_with_line(entt::registry& r, const entt::entity& e, const LineInfo& li);

void
set_position_and_size_with_line(entt::registry& r, const entt::entity& e, const LineInfo& li);

// inspired by:
// https://github.com/scikit-image/scikit-image/blob/main/skimage/draw/_draw.pyx
std::vector<std::pair<int, int>>
create_line(int r0, int c0, int r1, int c1);

std::optional<glm::ivec2>
line_intersection(const glm::ivec2& p0, const glm::ivec2& p1, const glm::ivec2& p2, const glm::ivec2& p3);

glm::ivec2
get_size_from_line(const Line& l, int line_width = 4);

glm::ivec2
get_center_from_line(const Line& l);

std::vector<std::pair<Line, glm::ivec2>>
get_line_collisions(const std::vector<Line>& lines, const Line& l);

bool
is_horizontal(const Line& l);

void
make_p1_bigger_point(Line& l);

} // game2d