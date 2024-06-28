#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct ShowInventoryRequest
{
  bool placeholder = true;
};

enum class InventorySlotType
{
  // body slots
  head,

  // general
  backpack,
};

struct InventorySlot
{
  InventorySlotType type = InventorySlotType::backpack;

  std::vector<entt::entity> items;
};

struct Item
{
  entt::entity parent = entt::null;
};

} // namespace game2d