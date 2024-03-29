#pragma once

#include <vector>

namespace game2d {

// e.g. an "Enemy" can have 100 "xp"

// two options here for enemies:
// a) the enemy has a level based on the xp it has
//    and the player gains all of the xp the enemy had?
// b) the enemy has a level independent of the xp it drops

struct XpComponent
{
  int amount = 0;
};

struct StatsComponent
{
  int overall_level = 1;
  int con_level = 1; // impacts hp  (how?)
  int str_level = 1; // impacts dmg (how?)
  int agi_level = 1; // impacts def (how?)
};

// simulate() informing UI
struct WantsToLevelUp
{
  bool placeholder = true;
};

// UI informing simulate()
struct WantsToLevelStat
{
  std::vector<int> con;
  std::vector<int> str;
  std::vector<int> agi;
};

} // namespace game2d