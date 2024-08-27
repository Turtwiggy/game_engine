#pragma once

#include "core.hpp"

namespace game2d {

// struct ItemType
// {
//   BREACH_CHARGE, MEDKIT, SCRAP, GUN,
// };

struct DataBreachCharge : public EntityData
{
  DataBreachCharge()
  {
    icon = "WEAPON_GRENADE";
    sprite = "WEAPON_GRENADE";
  };
};

struct DataMedkit : public EntityData
{
  int heal_amount = 25;

  // MISSING ICON
  // MISSING SPRITE
};

struct DataScrap : public EntityData
{
  DataScrap()
  {
    icon = "SCRAP";
    sprite = "SCRAP";
  }
};

} // namespace game2d