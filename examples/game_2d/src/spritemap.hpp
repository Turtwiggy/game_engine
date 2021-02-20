#pragma once

#include <map>
#include <vector>

#include <glm/glm.hpp>

#include "thirdparty/magic_enum.hpp"

namespace game2d {

namespace sprite {

enum class type
{
  BOAT,
  SQUARE,
};

struct spritemap
{
  inline std::map<type, glm::ivec2>& get_locations()
  {
    static std::map<type, glm::ivec2> ret;

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