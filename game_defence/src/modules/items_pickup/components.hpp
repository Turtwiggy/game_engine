#pragma once

//
// item/inventory
//

namespace game2d {

// attached to entity
struct WantsToPickUp
{
  bool placeholder = true;
};

// attached to item
struct AbleToBePickedUp
{
  bool placeholder = true;
};

} // namespace game2d