#include "common.hpp"
using namespace game2d::tests;

#include "actors/actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "lifecycle/system.hpp"
#include "maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_damage/system.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/system_turnbased_enemy/helpers.hpp"
#include "modules/ui_inventory/components.hpp"
using namespace game2d;

#include <entt/entt.hpp>
#include <fmt/core.h>
#include <gtest/gtest.h>

#include <algorithm>

TEST(TestMovement, MoveToFreeTile)
{
  // arrange
  entt::registry r;
  default_setup(r);
  const auto dungeon_e = Factory_DataDungeonActor::create(r, {});

  // act
  const auto& map = get_first_component<MapComponent>(r);
  const auto next_idx = engine::grid::grid_position_to_index({ 1, 0 }, map.xmax);
  auto next_wp = engine::grid::index_to_world_position_center(next_idx, map.xmax, map.ymax, map.tilesize);
  move_action_common(r, dungeon_e, next_wp);

  // assert: has moved
  const auto mapinfo = get_entity_mapinfo(r, dungeon_e);
  const int expected = 1;
  const int actual = mapinfo.value().idx_in_map;
  ASSERT_EQ(expected, actual);
};

TEST(TestMovement, MoveToTileOccupiedWithDungeonActor)
{
  // arrange
  entt::registry r;
  default_setup(r);
  const auto& map = get_first_component<MapComponent>(r);

  DataDungeonActor desc_0;
  desc_0.pos = engine::grid::index_to_world_position_center(0, map.xmax, map.ymax, map.tilesize);
  const auto dungeon_e_0 = Factory_DataDungeonActor::create(r, desc_0);

  DataDungeonActor desc_1;
  desc_1.pos = engine::grid::index_to_world_position_center(1, map.xmax, map.ymax, map.tilesize);
  const auto dungeon_e_1 = Factory_DataDungeonActor::create(r, desc_1);

  // act
  const auto next_idx = engine::grid::grid_position_to_index({ 1, 0 }, map.xmax);
  auto next_wp = engine::grid::index_to_world_position(next_idx, map.xmax, map.ymax, map.tilesize);
  next_wp += glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  move_action_common(r, dungeon_e_0, next_wp);

  // assert: has not moved
  const auto mapinfo = get_entity_mapinfo(r, dungeon_e_0);
  const int expected = 0;
  const int actual = mapinfo.value().idx_in_map;
  ASSERT_EQ(expected, actual);
};

TEST(TestMovement, MoveToTileOccupiedWithInventory)
{
  // arrange
  entt::registry r;
  default_setup(r);
  const auto& map = get_first_component<MapComponent>(r);

  // add dungeon actor
  entt::entity dungeon_e = entt::null;
  {
    DataDungeonActor desc;
    desc.pos = engine::grid::index_to_world_position_center(0, map.xmax, map.ymax, map.tilesize);
    desc.team = AvailableTeams::player;
    dungeon_e = Factory_DataDungeonActor::create(r, desc);
    r.emplace<DefaultBody>(dungeon_e, DefaultBody(r));
    r.emplace<DefaultInventory>(dungeon_e, DefaultInventory(r, 6 * 5));
    r.emplace<PlayerComponent>(dungeon_e);
  }

  // add inv to floor
  const auto pos = engine::grid::index_to_world_position_center(1, map.xmax, map.ymax, map.tilesize);
  DataDungeonLootbag loot_desc(DefaultInventory(r, 6 * 5));
  loot_desc.pos = pos;
  Factory_DataDungeonLootbag::create(r, loot_desc);

  // act
  const auto next_idx = engine::grid::grid_position_to_index({ 1, 0 }, map.xmax);
  auto next_wp = engine::grid::index_to_world_position_center(next_idx, map.xmax, map.ymax, map.tilesize);
  move_action_common(r, dungeon_e, next_wp);

  // assert: you have moved (its valid to move on to an inventory)
  const auto mapinfo = get_entity_mapinfo(r, dungeon_e);
  const int expected = 1;
  const int actual = mapinfo.value().idx_in_map;
  ASSERT_EQ(expected, actual);
};

TEST(TestMovement, MoveDungeonActorOverInventory)
{
  // arrange
  entt::registry r;
  default_setup(r);
  const auto& map = get_first_component<MapComponent>(r);

  // add dungeon actor
  entt::entity dungeon_e = entt::null;
  DataDungeonActor desc;
  desc.pos = engine::grid::index_to_world_position_center(0, map.xmax, map.ymax, map.tilesize);
  desc.team = AvailableTeams::player;
  dungeon_e = Factory_DataDungeonActor::create(r, desc);
  r.emplace<DefaultBody>(dungeon_e, DefaultBody(r));
  r.emplace<DefaultInventory>(dungeon_e, DefaultInventory(r, 6 * 5));
  r.emplace<PlayerComponent>(dungeon_e);

  // add inv to floor
  const auto pos = engine::grid::index_to_world_position_center(1, map.xmax, map.ymax, map.tilesize);
  DataDungeonLootbag loot_desc(DefaultInventory(r, 6 * 5));
  loot_desc.pos = pos;
  Factory_DataDungeonLootbag::create(r, loot_desc);

  // act 1: move dungeon actor on to inventory
  {
    const auto next_idx = engine::grid::grid_position_to_index({ 1, 0 }, map.xmax);
    auto next_wp = engine::grid::index_to_world_position_center(next_idx, map.xmax, map.ymax, map.tilesize);
    move_action_common(r, dungeon_e, next_wp);
  }

  // Assert: index 1 contains inventory and player (i.e. size=2)
  ASSERT_EQ(2, map.map[1].size());

  // act 2: move dungeon actor off inventory
  {
    const auto next_idx = engine::grid::grid_position_to_index({ 2, 0 }, map.xmax);
    auto next_wp = engine::grid::index_to_world_position_center(next_idx, map.xmax, map.ymax, map.tilesize);
    move_action_common(r, dungeon_e, next_wp);
  }

  // Assert: index 0 is empty (i.e. size=0)
  ASSERT_EQ(0, map.map[0].size());

  // Assert: index 1 contains inventory (i.e. size=1)
  ASSERT_EQ(1, map.map[1].size());

  // Assert: index 2 contains dungeon player (i.e. size=1)
  ASSERT_EQ(1, map.map[2].size());
}

TEST(TestMovement, DungeonActorCanDropInventoryWithNoItems)
{
  // arrange
  entt::registry r;
  default_setup(r);
  const auto& map = get_first_component<MapComponent>(r);

  DataDungeonActor desc_0;
  desc_0.pos = engine::grid::index_to_world_position_center(0, map.xmax, map.ymax, map.tilesize);
  const auto attacker_e = Factory_DataDungeonActor::create(r, desc_0);
  r.emplace<AttackComponent>(attacker_e, 10.0f); // comp could be on shotgun

  DataDungeonActor desc;
  desc.pos = engine::grid::index_to_world_position_center(1, map.xmax, map.ymax, map.tilesize);
  desc.hp = 10;
  desc.max_hp = 10;
  desc.team = AvailableTeams::enemy;
  const auto defender_e = Factory_DataDungeonActor::create(r, desc);

  DealDamageRequest req;
  req.from = attacker_e;
  req.to = defender_e;
  create_empty<DealDamageRequest>(r, req);

  // act: kill the entity
  update_take_damage_system(r);

  // assert: entity is dead
  const auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  const auto it = std::find(dead.dead.begin(), dead.dead.end(), defender_e);
  ASSERT_TRUE(it != dead.dead.end()); // i.e. it exists in the dead pile

  // act: tick. which is where the item-drop callback is called
  update_lifecycle_system(r, 10);

  // assert: item is dropped
  const auto& map_es = map.map[1];
  const auto map_e = map_es[0];

  // ASSERT_TRUE here doesnt matter, but the .get<> not throwing excepts does
  ASSERT_TRUE(r.get<DefaultInventory>(map_e).inv.size() > 0);
};
