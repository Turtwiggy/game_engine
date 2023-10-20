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

struct ItemComponent
{
  int item_id = 0;
};

} // namespace game2d