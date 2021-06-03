#pragma once

// standard lib
#include <map>
#include <stdexcept>
#include <vector>

// other proj headers
#include <glm/glm.hpp>

namespace game2d {

namespace sprite {

enum class type
{
  // row 0
  EMPTY,

  // row 1
  TREE_1,
  TREE_2,
  TREE_3,
  TREE_4,
  TREE_5,
  TREE_6,
  TREE_7,
  TREE_8,

  // row 3
  WALL_BIG,

  // row 5
  SQUARE,

  // row 15
  SKULL_AND_BONES,

  // row 19
  BOAT,

  // row 21
  SPACE_VEHICLE_1,
  SPACE_VEHICLE_2,
  SPACE_VEHICLE_3,
  FIREWORK,
  ROCKET_1,
  ROCKET_2
};

struct spritemap
{
  static inline std::map<type, glm::ivec2>& get_locations()
  {
    static std::map<type, glm::ivec2> ret;

    ret[type::EMPTY] = { 0, 0 };

    ret[type::TREE_1] = { 0, 1 };
    ret[type::TREE_2] = { 1, 1 };
    ret[type::TREE_3] = { 2, 1 };
    ret[type::TREE_4] = { 3, 1 };
    ret[type::TREE_5] = { 4, 1 };
    ret[type::TREE_6] = { 5, 1 };
    ret[type::TREE_7] = { 6, 1 };
    ret[type::TREE_8] = { 7, 1 };

    ret[type::WALL_BIG] = { 2, 3 };

    // add_to(ret[BASE], {0, 0});
    ret[type::SQUARE] = { 8, 5 };

    ret[type::SKULL_AND_BONES] = { 0, 15 };

    ret[type::BOAT] = { 10, 19 };

    ret[type::SPACE_VEHICLE_1] = { 12, 21 };
    ret[type::SPACE_VEHICLE_2] = { 13, 21 };
    ret[type::SPACE_VEHICLE_3] = { 14, 21 };
    ret[type::FIREWORK] = { 32, 21 };
    ret[type::ROCKET_1] = { 33, 21 };
    ret[type::ROCKET_2] = { 34, 21 };

    return ret;
  }

  static inline glm::vec2 get_sprite_offset(type t)
  {
    auto& tiles = get_locations();

    auto& which = tiles[t];

    return tiles[t];
  }
};

} // namespace sprite

} // namespace game2d