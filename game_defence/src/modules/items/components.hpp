#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <vector>

//
// item/inventory
//

namespace game2d {

struct WantsToPickUp
{
  bool placeholder = true;
};

struct WantsToDrop
{
  std::vector<entt::entity> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToDrop, items);
};

struct PickupZone
{
  bool placeholder = true;
};

} // namespace game2d