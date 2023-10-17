#pragma once

namespace game2d {

struct BaseTurretStatsComponent
{
  int overall_level = 1;
  int con_level = 1; // impacts hp  (how?)
  int str_level = 1; // impacts dmg (how?)
  int agi_level = 1; // impacts def (how?)

  int enemies_to_block = 1;

  float generate_ammo_per_second = 0;
};

} // namespace game2d