#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <vector>

//
// item/inventory
//

namespace game2d {

struct AbleToDropItem
{
  bool placeholder = true;
};

// attached to entity
struct WantsToDrop
{
  std::vector<entt::entity> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToDrop, items);
};

} // namespace game2d