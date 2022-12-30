#pragma once

#include "modules/dungeon/components.hpp"

#include <glm/glm.hpp>

namespace game2d {

constexpr glm::ivec2
room_center(const Room& r)
{
  return { (r.x1 + r.x2) / 2, (r.y1 + r.y2) / 2 };
};

bool
rooms_overlap(const Room& r0, const Room& r1);

} // namespace game2d