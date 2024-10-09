#pragma once

#include "modules/map/components.hpp"
#include <entt/entt.hpp>

namespace game2d {

struct AirlockComponent
{
  entt::entity mob_in_airlock = entt::null;

  Edge north_edge_copy;
  Edge south_edge_copy;

  entt::entity door_north;
  entt::entity door_south;
};

} // namespace game2d