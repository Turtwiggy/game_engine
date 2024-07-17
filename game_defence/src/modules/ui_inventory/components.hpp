#pragma once

#include <entt/entt.hpp>

#include <map>
#include <unordered_map>

namespace game2d {

struct ShowInventoryRequest
{
  bool placeholder = true;
};

enum class InventorySlotType
{
  // body slots
  head,
  chest,
  l_arm,
  r_arm,
  legs,
  feet,
  gun,

  // inventory
  backpack,
};

enum class ItemType
{
  scrap,
  scrap_helmet,
  scrap_chestpiece,
  scrap_arm_bracer,
  scrap_legs,
  scrap_boots,

  scrap_shotgun,
};

struct InventorySlotComponent
{
  InventorySlotType type = InventorySlotType::backpack;

  // parent <=> child
  entt::entity child_item = entt::null;

  auto operator<=>(const InventorySlotComponent&) const = default;
};

struct ItemComponent
{
  ItemType type = ItemType::scrap;

  // parent <=> child
  entt::entity parent_slot = entt::null;

  auto operator<=>(const ItemComponent&) const = default;
};

const static std::unordered_map<ItemType, std::vector<InventorySlotType>> item_to_slot_map{
  { ItemType::scrap, { InventorySlotType::backpack } },
  { ItemType::scrap_helmet, { InventorySlotType::head, InventorySlotType::backpack } },
  { ItemType::scrap_chestpiece, { InventorySlotType::chest, InventorySlotType::backpack } },
  { ItemType::scrap_arm_bracer, { InventorySlotType::l_arm, InventorySlotType::r_arm, InventorySlotType::backpack } },
  { ItemType::scrap_legs, { InventorySlotType::legs, InventorySlotType::backpack } },
  { ItemType::scrap_boots, { InventorySlotType::feet, InventorySlotType::backpack } },
};

} // namespace game2d