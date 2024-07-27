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
  core,
  arm,
  leg,

  // inventory
  gun,
  bullet,
  backpack,
};

enum class ItemType
{
  scrap,
  scrap_head_protection,
  scrap_core_protection,
  scrap_arm_protection,
  scrap_leg_protection,

  scrap_shotgun,

  bullettype_default,
  bullettype_bouncy,
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

// const static std::unordered_map<ItemType, std::vector<InventorySlotType>> item_to_slot_map{
//   { ItemType::scrap, { InventorySlotType::backpack } },
//   { ItemType::scrap_helmet, { InventorySlotType::head, InventorySlotType::backpack } },
//   { ItemType::scrap_chestpiece, { InventorySlotType::chest, InventorySlotType::backpack } },
//   { ItemType::scrap_arm_bracer, { InventorySlotType::l_arm, InventorySlotType::r_arm, InventorySlotType::backpack } },
//   { ItemType::scrap_legs, { InventorySlotType::legs, InventorySlotType::backpack } },
// };

const static std::unordered_map<ItemType, std::string> item_to_sprite_map{
  { ItemType::scrap, "SCRAP" },
  { ItemType::scrap_head_protection, "HELMET_5" },
  { ItemType::scrap_core_protection, "CHESTPIECE_5" },
  { ItemType::scrap_arm_protection, "GLOVES_4" },
  { ItemType::scrap_leg_protection, "BOOTS_2" },
  { ItemType::scrap_shotgun, "WEAPON_SHOTGUN" },
  { ItemType::bullettype_default, "AMMO_BOX" },
  { ItemType::bullettype_bouncy, "AMMO_BOX" },
};

} // namespace game2d