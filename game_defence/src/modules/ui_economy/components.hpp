#pragma once

#include "modules/combat/components.hpp"

namespace game2d {

struct SINGLETON_Economy
{
  int kills = 0;
};

struct SINGLETON_UiEconomy
{
  bool add_weapon = false;
  Weapon weapon_to_add;
};

} // namespace game2d