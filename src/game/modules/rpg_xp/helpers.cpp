#include "helpers.hpp"

namespace game2d {

int
convert_xp_to_level(const int xp)
{
  // 0xp will return lv1
  const int base_level = 1;
  const int base = 100;
  const int factor = 200;

  int level = base_level;

  // require the first level to have 150 xp
  int xp_accumulator = xp - base;
  if (xp_accumulator >= 0)
    level += 1;

  // require every level after to have 200 xp
  while ((xp_accumulator - factor) >= 0) {
    xp_accumulator -= factor;
    level += 1;
  }

  return level;
};

} // namespace game2d