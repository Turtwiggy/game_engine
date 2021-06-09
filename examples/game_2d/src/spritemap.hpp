#pragma once

// standard lib
#include <map>
#include <stdexcept>
#include <vector>

// other proj headers
#include <glm/glm.hpp>

// your proj headers
#include "engine/maths_core.hpp"

namespace game2d {

namespace sprite {

enum class type
{
  // row 0
  EMPTY,
  BUSH_0,
  BUSH_1,
  BUSH_2,
  BUSH_3,
  BUSH_4,
  BUSH_5,
  BUSH_6,
  BUSH_7,
  PERSON_0,
  PERSON_1,
  PERSON_2,
  PERSON_3,
  PERSON_4,
  PERSON_5,
  PERSON_6,
  PERSON_7,

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
  WEAPON_ARROW_1,
  WEAPON_ARROW_2,
  WEAPON_SHOVEL,

  // row 6
  ORC,

  // row 14
  CAMPFIRE,
  FIRE,

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

    // row 0
    ret[type::EMPTY] = { 0, 0 };
    ret[type::BUSH_0] = { 1, 0 };
    ret[type::BUSH_1] = { 2, 0 };
    ret[type::BUSH_2] = { 3, 0 };
    ret[type::BUSH_3] = { 4, 0 };
    ret[type::BUSH_4] = { 5, 0 };
    ret[type::BUSH_5] = { 6, 0 };
    ret[type::BUSH_6] = { 7, 0 };
    ret[type::PERSON_0] = { 24, 0 };
    ret[type::PERSON_1] = { 25, 0 };
    ret[type::PERSON_2] = { 26, 0 };
    ret[type::PERSON_3] = { 27, 0 };
    ret[type::PERSON_4] = { 28, 0 };
    ret[type::PERSON_5] = { 29, 0 };
    ret[type::PERSON_6] = { 30, 0 };
    ret[type::PERSON_7] = { 31, 0 };

    // row 1
    ret[type::TREE_1] = { 0, 1 };
    ret[type::TREE_2] = { 1, 1 };
    ret[type::TREE_3] = { 2, 1 };
    ret[type::TREE_4] = { 3, 1 };
    ret[type::TREE_5] = { 4, 1 };
    ret[type::TREE_6] = { 5, 1 };
    ret[type::TREE_7] = { 6, 1 };
    ret[type::TREE_8] = { 7, 1 };

    // row 3
    ret[type::WALL_BIG] = { 2, 3 };

    // row 5
    ret[type::SQUARE] = { 8, 5 };
    ret[type::WEAPON_ARROW_1] = { 40, 5 };
    ret[type::WEAPON_ARROW_2] = { 41, 5 };
    ret[type::WEAPON_SHOVEL] = { 42, 5 };

    // row 6
    ret[type::ORC] = { 30, 6 };

    // row 10
    ret[type::CAMPFIRE] = { 14, 10 };
    ret[type::FIRE] = { 15, 10 };

    // row 15
    ret[type::SKULL_AND_BONES] = { 0, 15 };

    // row 19
    ret[type::BOAT] = { 10, 19 };

    // row 21
    ret[type::SPACE_VEHICLE_1] = { 12, 21 };
    ret[type::SPACE_VEHICLE_2] = { 13, 21 };
    ret[type::SPACE_VEHICLE_3] = { 14, 21 };
    ret[type::FIREWORK] = { 32, 21 };
    ret[type::ROCKET_1] = { 33, 21 };
    ret[type::ROCKET_2] = { 34, 21 };

    return ret;
  }

  static inline std::map<type, float>& get_rotations()
  {
    static std::map<type, float> ret;

    // row 5
    ret[type::SQUARE] = { 0.0f };
    ret[type::WEAPON_ARROW_1] = { -fightingengine::PI / 4.0f };
    ret[type::WEAPON_ARROW_2] = { -fightingengine::PI / 4.0f };
    ret[type::WEAPON_SHOVEL] = { -fightingengine::PI / 4.0f };

    return ret;
  }

  static inline glm::vec2 get_sprite_offset(const type& t)
  {
    auto& tiles = get_locations();

    return tiles[t];
  };

  static inline float get_sprite_rotation_offset(const type& t)
  {
    auto& tiles = get_rotations();

    return tiles[t];
  };
};

} // namespace sprite

} // namespace game2d