#pragma once

// your proj headers
#include "modules/editor_hex_map/components.hpp"

// other proj headers
#include <glm/glm.hpp>

namespace game2d {

glm::ivec3
convert_world_pos_to_hex_pos(const glm::ivec2& world_pos, const glm::ivec2& offset);

glm::vec2
convert_hex_pos_to_world_pos(const glm::ivec3& hex_pos, const glm::ivec2& offset);

enum class HexDirection
{
  NE,
  E,
  SE,
  SW,
  W,
  NW
};

// inline std::reference_wrapper<HexCell>
// get_neighbour(HexData& data, const HexDirection& dir)
// {
//   return data.neighbours[static_cast<int>(dir)];
// };

// inline void
// set_neighbour(HexData& data, const HexDirection& dir, const std::reference_wrapper<HexCell>& cell;)
// {
//   data.neighbours[static_cast<int>(dir)] = cell;
//   cell.neighbours[static_cast<int>(dir)] = this;
// };

} // namespace game2d