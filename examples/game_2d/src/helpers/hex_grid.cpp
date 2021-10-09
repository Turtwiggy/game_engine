// your header
#include "helpers/hex_grid.hpp"

// c++ lib headers
#include <iostream>

namespace game2d {

glm::ivec3
convert_world_pos_to_hex_pos(const glm::ivec2& world_pos, const glm::ivec2& offset)
{
  glm::ivec2 offset_world_pos = world_pos - offset;

  float fx = offset_world_pos.x / (inner_radius * 2.0f);
  float fz = -fx;
  const float y_offset = offset_world_pos.y / (outer_radius * 3.0f);
  fx -= y_offset;
  fz -= y_offset;
  int ix = int(glm::roundEven(fx));
  int iy = int(glm::roundEven(-fx - fz));
  int iz = int(glm::roundEven(fz));

  const int sum_coords = ix + iy + iz;
  if (sum_coords != 0) {
    // the coordinate that got the most rounded is incorrect.
    const float dx = glm::abs(fx - ix);
    const float dy = glm::abs(-fx - fz - iy);
    const float dz = glm::abs(fz - iz);

    if (dx > dy && dx > dz) {
      ix = -iy - iz;
    } else if (dy > dx || dy > dz) {
      iy = -iz - ix;
    } else if (dz > dx || dz > dy) {
      iz = -iy - ix;
    }

    const int new_sum_coords = ix + iy + iz;
    if (new_sum_coords != 0) {
      std::cerr << "Error: grid does not add up to 0?" << std::endl;
    }
  }

  return { ix, iy, iz };
};

glm::vec2
convert_hex_pos_to_world_pos(const glm::ivec3& hex_pos, const glm::ivec2& offset)
{
  return glm::vec2(0.0f, 0.0f) + glm::vec2(offset.x, offset.y);
};

} // namespace game2d