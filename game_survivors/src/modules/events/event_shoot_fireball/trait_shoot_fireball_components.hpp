#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct TraitFireballComponent
{
  int shots_until_fireball_cur = 5;
  int shots_until_fireball_max = 5;
};

} // namespace game2d