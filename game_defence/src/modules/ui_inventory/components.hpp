#pragma once

#include "engine/entt/helpers.hpp"
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
  core,
  arm,
  leg,

  // inventory
  gun,
  bullet,
  backpack,
};

struct InventorySlotComponent
{
  InventorySlotType type = InventorySlotType::head;

  // parent <=> child
  entt::entity item_e = entt::null;

  auto operator<=>(const InventorySlotComponent&) const = default;
};

struct ItemComponent
{
  std::string display_icon = "EMPTY";
  std::string display_name = "DISPLAY_NAME";

  // parent <=> child
  entt::entity parent_slot = entt::null;

  auto operator<=>(const ItemComponent&) const = default;
};

// where is item allowed to go?
struct AllowedSlotsComponent
{
  //   { ItemType::scrap, { InventorySlotType::backpack } },
  //   { ItemType::scrap_helmet, { InventorySlotType::head, InventorySlotType::backpack } },
  //   { ItemType::scrap_chestpiece, { InventorySlotType::chest, InventorySlotType::backpack } },
  //   { ItemType::scrap_arm_bracer, { InventorySlotType::l_arm, InventorySlotType::r_arm, InventorySlotType::backpack } },
  //   { ItemType::scrap_legs, { InventorySlotType::legs, InventorySlotType::backpack } },
  std::vector<InventorySlotType> slots;
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

// Items that have actions
//
// struct ItemMedkitActionComponent
// {
//   int heal_amount = 25;
// };

struct SINGLE_UIInventoryState
{
  std::vector<std::string> hovered_buttons;
};

} // namespace game2d