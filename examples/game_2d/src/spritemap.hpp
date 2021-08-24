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
  CASTLE_FLOOR,

  // row 3
  WALL_BIG,

  // row 5
  SQUARE,
  WEAPON_ARROW_1,
  WEAPON_ARROW_2,
  WEAPON_SHOVEL,
  WEAPON_PICKAXE,

  // row 6
  ORC,
  WEAPON_DAGGER_OUTLINED_1,
  WEAPON_DAGGER_SOLID_3,

  // row 9
  WEAPON_PISTOL,
  WEAPON_RAY_PISTOL,
  WEAPON_SHOTGUN,
  WEAPON_SUB_MACHINE_GUN,
  WEAPON_MINI_UZI,
  WEAPON_MP5,
  WEAPON_RPG,
  AMMO_BOX,

  // row 10
  CAMPFIRE,
  FIRE,
  ICON_HEART,
  ICON_HEART_OUTLINE,
  ICON_HEART_HALF_FULL,
  ICON_HEART_FULL,

  // row 15
  SKULL_AND_BONES,

  // row 17
  NUMBER_0,
  NUMBER_1,
  NUMBER_2,
  NUMBER_3,
  NUMBER_4,
  NUMBER_5,
  NUMBER_6,
  NUMBER_7,
  NUMBER_8,
  NUMBER_9,

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
    ret[type::CASTLE_FLOOR] = { 19, 1 };

    // row 3
    ret[type::WALL_BIG] = { 2, 3 };

    // row 5
    ret[type::SQUARE] = { 8, 5 };
    ret[type::WEAPON_ARROW_1] = { 40, 5 };
    ret[type::WEAPON_ARROW_2] = { 41, 5 };
    ret[type::WEAPON_SHOVEL] = { 42, 5 };
    ret[type::WEAPON_PICKAXE] = { 42, 5 };

    // row 6
    ret[type::ORC] = { 30, 6 };
    ret[type::WEAPON_DAGGER_OUTLINED_1] = { 32, 6 };
    ret[type::WEAPON_DAGGER_SOLID_3] = { 34, 6 };

    // row 9
    ret[type::WEAPON_PISTOL] = { 37, 9 };
    ret[type::WEAPON_RAY_PISTOL] = { 38, 9 };
    ret[type::WEAPON_SHOTGUN] = { 39, 9 };
    ret[type::WEAPON_SUB_MACHINE_GUN] = { 40, 9 };
    ret[type::WEAPON_MINI_UZI] = { 41, 9 };
    ret[type::WEAPON_MP5] = { 42, 9 };
    ret[type::WEAPON_RPG] = { 43, 9 };
    ret[type::AMMO_BOX] = { 44, 9 };

    // row 10
    ret[type::CAMPFIRE] = { 14, 10 };
    ret[type::FIRE] = { 15, 10 };
    ret[type::ICON_HEART] = { 39, 10 };
    ret[type::ICON_HEART_OUTLINE] = { 40, 10 };
    ret[type::ICON_HEART_HALF_FULL] = { 41, 10 };
    ret[type::ICON_HEART_FULL] = { 42, 10 };

    // row 15
    ret[type::SKULL_AND_BONES] = { 0, 15 };

    // row 17
    ret[type::NUMBER_0] = { 35, 17 };
    ret[type::NUMBER_1] = { 36, 17 };
    ret[type::NUMBER_2] = { 37, 17 };
    ret[type::NUMBER_3] = { 38, 17 };
    ret[type::NUMBER_4] = { 39, 17 };
    ret[type::NUMBER_5] = { 40, 17 };
    ret[type::NUMBER_6] = { 41, 17 };
    ret[type::NUMBER_7] = { 42, 17 };
    ret[type::NUMBER_8] = { 43, 17 };
    ret[type::NUMBER_9] = { 44, 17 };

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
    ret[type::WEAPON_PICKAXE] = { -fightingengine::PI / 8.0f };

    // row 6
    ret[type::WEAPON_DAGGER_OUTLINED_1] = { -fightingengine::PI / 4.0f };
    ret[type::WEAPON_DAGGER_SOLID_3] = { -fightingengine::PI / 4.0f };

    // row 9
    ret[type::WEAPON_PISTOL] = { -fightingengine::PI / 2.0f };
    ret[type::WEAPON_RAY_PISTOL] = { -fightingengine::PI / 2.0f };
    ret[type::WEAPON_SHOTGUN] = { -fightingengine::PI / 2.0f };
    ret[type::WEAPON_SUB_MACHINE_GUN] = { -fightingengine::PI / 2.0f };
    ret[type::WEAPON_MINI_UZI] = { -fightingengine::PI / 2.0f };
    ret[type::WEAPON_MP5] = { -fightingengine::PI / 2.0f };
    ret[type::WEAPON_RPG] = { -fightingengine::PI / 2.0f };

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