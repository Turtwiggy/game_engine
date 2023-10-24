#pragma once

#include "sprites/components.hpp"

#include <entt/entt.hpp>

#include <string>

namespace game2d {

struct ItemInfo
{
  std::string display;
  std::string sprite = "EMPTY";
  std::string sprite_text = "EMPTY";
};

[[nodiscard]] ItemInfo
item_id_to_sprite(entt::registry& r, const int item_id);

} // namespace game2d