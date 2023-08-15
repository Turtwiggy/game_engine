#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <vector>

//
// item/inventory
//

namespace game2d {

// attached to entity
struct WantsToPickUp
{
  bool placeholder = true;
};

// attached to entity
struct WantsToDrop
{
  std::vector<entt::entity> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToDrop, items);
};

struct PickupZone
{
  bool placeholder = true;
};

struct AbleToBePickedUp
{
  bool placeholder = true;
};

} // namespace game2d