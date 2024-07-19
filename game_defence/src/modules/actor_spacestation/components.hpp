#pragma once

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

} // namespace game2d