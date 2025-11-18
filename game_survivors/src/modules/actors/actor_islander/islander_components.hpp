#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct IslandDwellerComponent
{
  bool placeholder = true;
};

struct DroppedAnchorComponent
{
  entt::entity target_e = entt::null;
};

} // namespace game2d