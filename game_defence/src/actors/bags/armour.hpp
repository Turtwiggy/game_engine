#pragma once

#include "actors/bags/core.hpp"

namespace game2d {

enum class ArmourType
{
  HEAD,
  CORE,
  ARM,
  LEG,
};

enum class ArmourFaction
{
  HUMAN,
};

struct ArmourData
{
  ArmourType type = ArmourType::HEAD;
  ArmourFaction faction = ArmourFaction::HUMAN;
  int armour_amount = 1;
};

struct DataArmour
  : public EntityData
  , public ArmourData
{
  DataArmour() = delete;
  DataArmour(const ArmourType& in_type)
  {
    faction = ArmourFaction::HUMAN;
    type = in_type;

    switch (type) {
      case ArmourType::HEAD: {
        icon = "HELMET_5";
        sprite = "HELMET_5";
        break;
      }
      case ArmourType::CORE: {
        icon = "CHESTPIECE_5";
        sprite = "CHESTPIECE_5";
        break;
      }
      case ArmourType::ARM: {
        icon = "GLOVES_4";
        sprite = "GLOVES_4";
        break;
      }
      case ArmourType::LEG: {
        icon = "BOOTS_2";
        sprite = "BOOTS_2";
        break;
      }
    }
  };
};

} // namespace game2d