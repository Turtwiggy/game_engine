#include "actors/bags/armour.hpp"
#include "common.hpp"
#include "modules/actor_armour/components.hpp"
using namespace game2d::tests;

#include "actors/actors.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"
using namespace game2d;

#include <entt/entt.hpp>
#include <gtest/gtest.h>

//

TEST(TestInventory, MoveInventoryItemToFreeInventorySlotOnSelf)
{
  // arrange
  entt::registry r;
  default_setup(r);

  // dungeon entity with body and inventory
  DataDungeonActor desc;
  desc.team = AvailableTeams::enemy;
  const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);
  create_inv_scrap(r, r.get<DefaultInventory>(dungeon_e).inv[0]);

  const auto full_slot_e = r.get<DefaultInventory>(dungeon_e).inv[0];
  const auto& full_slot_c = r.get<InventorySlotComponent>(full_slot_e);
  const auto free_slot_e = r.get<DefaultInventory>(dungeon_e).inv[1];
  const auto& free_slot_c = r.get<InventorySlotComponent>(free_slot_e);

  // act
  handle_dragdrop_target(r, full_slot_c.item_e, free_slot_e);

  // assert: item is gone from dungeon entity
  ASSERT_TRUE(full_slot_c.item_e == entt::null);

  // assert: item has gone to the lootbag's inventory
  ASSERT_TRUE(free_slot_c.item_e != entt::null);
};

// note: should be fine. the items should swap.
TEST(TestInventory, MoveInventoryItemToFullInventorySlotOnSelf)
{
  // arrange
  entt::registry r;
  default_setup(r);

  // dungeon entity with body and inventory
  DataDungeonActor desc;
  desc.team = AvailableTeams::enemy;
  const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);
  create_inv_scrap(r, r.get<DefaultInventory>(dungeon_e).inv[0]);
  create_inv_armour(r, r.get<DefaultInventory>(dungeon_e).inv[1], ArmourType::ARM);

  const auto full_slot_0_e = r.get<DefaultInventory>(dungeon_e).inv[0];
  const auto full_slot_1_e = r.get<DefaultInventory>(dungeon_e).inv[1];
  const auto& full_slot_0_c = r.get<InventorySlotComponent>(full_slot_0_e);
  const auto& full_slot_1_c = r.get<InventorySlotComponent>(full_slot_1_e);

  // act
  handle_dragdrop_target(r, full_slot_0_c.item_e, full_slot_1_e);

  // assert: items still exist
  ASSERT_TRUE(full_slot_0_c.item_e != entt::null);
  ASSERT_TRUE(full_slot_0_c.item_e != entt::null);

  // assert: slot 0 is now armour
  ASSERT_TRUE(r.get<ItemTypeComponent>(full_slot_0_c.item_e).type == ItemType::armour);

  // assert: slot 1 is now scrap
  ASSERT_TRUE(r.get<ItemTypeComponent>(full_slot_1_c.item_e).type == ItemType::scrap);
};

TEST(TestInventory, MoveInventoryItemToFreeEquipmentSlotOnSelf)
{
  // arrange
  entt::registry r;
  default_setup(r);

  // dungeon entity with body and inventory
  DataDungeonActor desc;
  desc.team = AvailableTeams::enemy;
  const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);
  auto& inv = r.get<DefaultInventory>(dungeon_e);
  create_inv_scrap(r, inv.inv[0]);
  create_inv_scrap(r, inv.inv[1]);
  create_inv_scrap(r, inv.inv[2]);
  create_inv_scrap(r, inv.inv[3]);
  create_inv_scrap(r, inv.inv[4]);
  create_inv_armour(r, inv.inv[5], ArmourType::ARM);

  const int idx_to_move = 3;
  const auto full_slot_e = r.get<DefaultInventory>(dungeon_e).inv[idx_to_move];
  const auto& full_slot_c = r.get<InventorySlotComponent>(full_slot_e);
  const auto empty_bodyslot_e = r.get<DefaultBody>(dungeon_e).body[0];
  const auto& empty_bodyslot_c = r.get<InventorySlotComponent>(empty_bodyslot_e);

  // act: swap scrap into a body slot
  handle_dragdrop_target(r, full_slot_c.item_e, empty_bodyslot_e);

  // assert: bodyslot 0 is now scrap
  ASSERT_TRUE(r.get<ItemTypeComponent>(empty_bodyslot_c.item_e).type == ItemType::scrap);

  // assert: inventory slot is now empty
  ASSERT_TRUE(full_slot_c.item_e == entt::null);

  // assert: inventory slot is untouched (still armour)
  const auto& armour_c = r.get<InventorySlotComponent>(inv.inv[5]);
  ASSERT_TRUE(armour_c.item_e != entt::null);
  ASSERT_TRUE(r.get<ItemTypeComponent>(armour_c.item_e).type == ItemType::armour);
};

TEST(TestInventory, MoveInventoryItemToFullEquipmentSlotOnSelf)
{
  // arrange
  entt::registry r;
  default_setup(r);

  // dungeon entity with body and inventory
  DataDungeonActor desc;
  desc.team = AvailableTeams::enemy;
  const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);
  auto& inv = r.get<DefaultInventory>(dungeon_e);
  create_inv_armour(r, r.get<DefaultInventory>(dungeon_e).inv[0], ArmourType::ARM);
  create_inv_armour(r, r.get<DefaultBody>(dungeon_e).body[0], ArmourType::HEAD);

  const auto full_itemslot_e = r.get<DefaultInventory>(dungeon_e).inv[0];
  const auto& full_itemslot_c = r.get<InventorySlotComponent>(full_itemslot_e);
  const auto full_bodyslot_e = r.get<DefaultBody>(dungeon_e).body[0];
  const auto& full_bodyslot_c = r.get<InventorySlotComponent>(full_bodyslot_e);

  // act
  handle_dragdrop_target(r, full_itemslot_c.item_e, full_bodyslot_e);

  // assert: bodyslot 0 is now ARM
  ASSERT_TRUE(r.get<ArmourComponent>(full_bodyslot_c.item_e).data.type == ArmourType::ARM);

  // assert: inventoryslot 0 is now HEAD
  ASSERT_TRUE(full_itemslot_c.item_e != entt::null);
  ASSERT_TRUE(r.get<ArmourComponent>(full_itemslot_c.item_e).data.type == ArmourType::HEAD);
};

TEST(TestInventory, MoveInventoryItemToFullEquipmentSlotOnSelfAndBack)
{
  // arrange
  entt::registry r;
  default_setup(r);

  // dungeon entity with body and inventory
  DataDungeonActor desc;
  desc.team = AvailableTeams::enemy;
  const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);
  auto& inv = r.get<DefaultInventory>(dungeon_e);
  create_inv_armour(r, r.get<DefaultInventory>(dungeon_e).inv[0], ArmourType::ARM);
  create_inv_armour(r, r.get<DefaultBody>(dungeon_e).body[0], ArmourType::HEAD);

  const auto itemslot_0_e = r.get<DefaultInventory>(dungeon_e).inv[0];
  const auto& itemslot_0_c = r.get<InventorySlotComponent>(itemslot_0_e);
  const auto itemslot_1_e = r.get<DefaultBody>(dungeon_e).body[0];
  const auto& itemslot_1_c = r.get<InventorySlotComponent>(itemslot_1_e);

  // act: swap and swap back
  handle_dragdrop_target(r, itemslot_0_c.item_e, itemslot_1_e);
  handle_dragdrop_target(r, itemslot_1_c.item_e, itemslot_0_e);

  // assert
  const auto a = r.get<DefaultInventory>(dungeon_e).inv[0];
  const auto& a_c = r.get<InventorySlotComponent>(a);
  const auto b = r.get<DefaultBody>(dungeon_e).body[0];
  const auto& b_c = r.get<InventorySlotComponent>(b);
  ASSERT_EQ(r.get<ArmourComponent>(a_c.item_e).data.type, ArmourType::ARM);
  ASSERT_EQ(r.get<ArmourComponent>(b_c.item_e).data.type, ArmourType::HEAD);
}