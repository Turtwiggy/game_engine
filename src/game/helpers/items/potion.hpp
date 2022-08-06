#pragma once

#include "game/helpers/items/item.hpp"

#include <entt/entt.hpp>

namespace game2d {

struct Potion : public Item
{
  Potion();
  virtual bool use(entt::registry& r);
};

} // namespace game2d
