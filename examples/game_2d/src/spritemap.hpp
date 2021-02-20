#pragma once

#include <map>
#include <vector>

#include <glm/glm.hpp>

#include "thirdparty/magic_enum.hpp"

namespace game2d {

namespace sprite {

enum class type
{
  TREE_1,
  TREE_2,
  TREE_3,
  TREE_4,
  TREE_5,
  TREE_6,
  TREE_7,
  TREE_8,

  BOAT,
  SQUARE,

};

struct spritemap
{
  inline std::map<type, glm::ivec2>& get_locations()
  {
    static std::map<type, glm::ivec2> ret;

    ret[type::TREE_1] = { 0, 1 };
    ret[type::TREE_2] = { 1, 1 };
    ret[type::TREE_3] = { 2, 1 };
    ret[type::TREE_4] = { 3, 1 };
    ret[type::TREE_5] = { 4, 1 };
    ret[type::TREE_6] = { 5, 1 };
    ret[type::TREE_7] = { 6, 1 };
    ret[type::TREE_8] = { 7, 1 };

    // add_to(ret[BASE], {0, 0});
    ret[type::BOAT] = { 10, 19 };
    ret[type::SQUARE] = { 8, 5 };

    return ret;
  }

  inline glm::vec2 get_sprite_offset(type t)
  {
    auto& tiles = get_locations();

    auto& which = tiles[t];

    // if (which.size() == 0)
    //   throw std::runtime_error("No tiles for type " + std::string(magic_enum::enum_name(t)).c_str());

    return tiles[t];
  }
};

} // namespace sprite

} // namespace game2d