#pragma once

#include "entt/helpers.hpp"
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
  // resource
  scrap,

  // armour
  head_protection,
  core_protection,
  arm_protection,
  leg_protection,

  // weapons
  scrap_shotgun,
  breach_charge,

  bullettype_default,
  bullettype_bouncy,
};

struct InventorySlotComponent
{
  InventorySlotType type = InventorySlotType::backpack;

  // parent <=> child
  entt::entity item_e = entt::null;

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

// ui icons
const static std::unordered_map<ItemType, std::string> item_to_sprite_map{
  { ItemType::scrap, "SCRAP" },
  { ItemType::head_protection, "HELMET_5" },
  { ItemType::core_protection, "CHESTPIECE_5" },
  { ItemType::arm_protection, "GLOVES_4" },
  { ItemType::leg_protection, "BOOTS_2" },
  { ItemType::scrap_shotgun, "WEAPON_SHOTGUN" },
  { ItemType::bullettype_default, "AMMO_BOX" },
  { ItemType::bullettype_bouncy, "AMMO_BOX" },
  { ItemType::breach_charge, "WEAPON_GRENADE" },
};

struct DefaultBody
{
  std::vector<entt::entity> body;

  DefaultBody() = default;
  DefaultBody(entt::registry& r)
  {
    const auto create_body_slot = [&r](const InventorySlotType& type) -> entt::entity {
      return create_empty<InventorySlotComponent>(r, InventorySlotComponent{ type });
    };

    // body slots
    body.push_back(create_body_slot(InventorySlotType::head));
    body.push_back(create_body_slot(InventorySlotType::core));
    body.push_back(create_body_slot(InventorySlotType::arm));
    body.push_back(create_body_slot(InventorySlotType::arm));
    body.push_back(create_body_slot(InventorySlotType::leg));
    body.push_back(create_body_slot(InventorySlotType::leg));

    // equipment slots
    body.push_back(create_body_slot(InventorySlotType::gun));
    body.push_back(create_body_slot(InventorySlotType::bullet));
  }
};

struct DefaultInventory
{
  std::vector<entt::entity> inv;

  DefaultInventory() = default;
  DefaultInventory(entt::registry& r, int size)
  {
    inv.resize(size);
    for (int i = 0; i < size; i++)
      inv[i] = create_empty<InventorySlotComponent>(r, InventorySlotComponent{ InventorySlotType::backpack });
  }
};

struct InitBodyAndInventory
{
  bool placeholder = true;
};

} // namespace game2d