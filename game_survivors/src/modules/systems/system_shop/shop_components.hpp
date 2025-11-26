#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct Item_OnDiskData
{
  std::string key;
  bool default_unlocked;
  int cost;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Item_OnDiskData, key, default_unlocked, cost);
};

struct SINGLE_Shop
{
  std::vector<Item_OnDiskData> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_Shop, items);
};

} // namespace game2d