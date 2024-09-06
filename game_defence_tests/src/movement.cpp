#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "lifecycle/system.hpp"
#include "maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_damage/system.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/system_turnbased_enemy/helpers.hpp"
#include "modules/ui_inventory/components.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"
using namespace game2d;

#include <entt/entt.hpp>
#include <fmt/core.h>
#include <gtest/gtest.h>

#include <algorithm>

void
default_setup(entt::registry& r)
{
  // load spritesheet info
  SINGLETON_RendererInfo ri = get_default_rendererinfo();
  SINGLE_Animations anims;
  for (const auto& tex : ri.user_textures)
    load_sprites(anims, tex.spritesheet_path);
  create_empty<SINGLE_Animations>(r, anims);
  create_empty<SINGLETON_RendererInfo>(r, ri);

  // create a camera
  const auto camera_e = create_empty<OrthographicCamera>(r);
  r.emplace<TransformComponent>(camera_e);

  move_to_scene_start(r, Scene::test);

  MapComponent map;
  map.tilesize = 50;
  map.xmax = 10;
  map.ymax = 10;
  map.map.resize(map.xmax * map.ymax);
  r.emplace<MapComponent>(r.create(), map);
};

TEST(TestSuite, MoveToFreeTile)
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

TEST(TestSuite, MoveToTileOccupiedWithDungeonActor)
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

TEST(TestSuite, MoveToTileOccupiedWithInventory)
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
  DataDungeonLootbag loot_desc;
  loot_desc.pos = pos;
  loot_desc.inventory = DefaultInventory(r, 6 * 5);
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

TEST(TestSuite, MoveDungeonActorOverInventory)
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
  DataDungeonLootbag loot_desc;
  loot_desc.pos = pos;
  loot_desc.inventory = DefaultInventory(r, 6 * 5);
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

TEST(TestSuite, DungeonActorCanDropInventoryWithNoItems)
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
  const auto& inv_c = r.get<DefaultInventory>(map_e);

  const int expected = 30;
  const int actual = inv_c.inv.size();
  ASSERT_EQ(expected, actual);
};

/*

TEST(TestSuite, MoveToItemTileWithInventory)
{

  // scrap item on other tile...
  const auto item_e = Factory_DataScrap::create(r, {});
  add_entity_to_map(r, item_e, 1);

  // act
  {
    const auto& map = get_first_component<MapComponent>(r);
    const auto next_idx = engine::grid::grid_position_to_index({ 1, 0 }, map.xmax);
    auto next_wp = engine::grid::index_to_world_position(next_idx, map.xmax, map.ymax, map.tilesize);
    next_wp += glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
    move_action_common(r, dungeon_e, next_wp);
  }

  // assert: dungeon entity has moved
  {
    const auto& map = get_first_component<MapComponent>(r);
    const auto it = std::find(map.map.begin(), map.map.end(), dungeon_e);
    const auto idx = static_cast<int>(it - map.map.begin());
    const glm::ivec2 expected = glm::ivec2{ 1, 0 };
    const glm::ivec2 actual = engine::grid::index_to_grid_position(idx, map.xmax, map.ymax);
    ASSERT_EQ(expected, actual);
  }

  // assert: dungeon entity picked up item
  // note: currently on pickup, the grid-entity is destroyed,
  // and a new entity is created and added to inventory
  {
    const auto& inv = r.get<DefaultInventory>(dungeon_e);

    int items_found = 0;

    for (size_t i = 0; i < inv.inv.size(); i++) {
      const auto slot_e = inv.inv[i];
      const auto& slot_c = r.get<InventorySlotComponent>(slot_e);
      if (slot_c.item_e == entt::null)
        continue; // slot not free
      items_found++;
    }

    ASSERT_EQ(1, items_found);
  }

  // assert: item is no longer on the map
  {
    const auto& map = get_first_component<MapComponent>(r);
    const auto it = std::find(map.map.begin(), map.map.end(), item_e);
    ASSERT_TRUE(it == map.map.end());
  }
};

*/
