#pragma once

namespace game2d {

struct TurretComponent
{
  // after this cutoff, no longer target enemies
  // tilesize * tilesize * tiles
  int distance2_cutoff = 16 * 16 * 100;
};

} // namespace game2d