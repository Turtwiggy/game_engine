#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SpacestationComponent
{
  // more of a gameplay thing to identify each station via number
  int idx = 0;
};

struct SpacestationUndiscoveredComponent
{
  bool placeholder = true;
};

struct DockedAtStationComponent
{
  entt::entity station_e = entt::null;
};

} // namespace game2d