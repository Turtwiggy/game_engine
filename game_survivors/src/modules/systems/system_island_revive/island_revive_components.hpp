#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct RevivableComponent
{
  int hits_to_revive = 3;
  int hits_to_revive_cur = 0;
};

} // namespace game2d