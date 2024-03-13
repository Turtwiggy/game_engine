#pragma once

#include <entt/entt.hpp>

//
// item/inventory
//

namespace game2d {

struct WantsToPickUp
{
  bool placeholder = true;
};

struct AbleToBePickedUp
{
  bool placeholder = true;
};

struct PickedUpByComponent
{
  entt::entity entity;
};

struct ItemComponent
{
  bool placeholder = true;
};

} // namespace game2d