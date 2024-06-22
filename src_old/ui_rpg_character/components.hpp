#pragma once

#include <string>

namespace game2d {

struct CharacterStats
{
  std::string name = "default";

  int constitution = 10;
  int charisma = 10;
  int dexterity = 10;
  int strength = 10;
  int wisdom = 10;
  int intelligence = 10;

  // live combat data, shouldnt be here
  int initiative = 0;
  int hp = 10;
  int xp = 0;
};

struct CharacterPrefab
{
  bool placeholder = true;
};

struct InActiveFight
{
  bool placeholder = true;
};

struct DamageInstance
{
  int amount = 0;
};

} // namespace game2d