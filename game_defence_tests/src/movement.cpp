#include "actors/actors.hpp"
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

  MapComponent map_c;
  map_c.tilesize = 50;
  map_c.xmax = 10;
  map_c.ymax = 10;
  map_c.map.resize(map_c.xmax * map_c.ymax, entt::null);
  r.emplace<MapComponent>(r.create(), map_c);
};

TEST(TestSuite, MoveToFreeTile)
{
  // arrange
  entt::registry r;
  default_setup(r);
  const auto dungeon_e = Factory_DataDungeonActor::create(r, {});
  add_entity_to_map(r, dungeon_e, 0);

  // act
  const auto& map_c = get_first_component<MapComponent>(r);
  const auto next_idx = engine::grid::grid_position_to_index({ 1, 0 }, map_c.xmax);
  auto next_wp = engine::grid::index_to_world_position(next_idx, map_c.xmax, map_c.ymax, map_c.tilesize);
  next_wp += glm::vec2{ map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
  move_action_common(r, dungeon_e, next_wp);

  // assert: has moved
  const auto it = std::find(map_c.map.begin(), map_c.map.end(), dungeon_e);
  const auto idx = static_cast<int>(it - map_c.map.begin());
  const glm::ivec2 expected = glm::ivec2{ 1, 0 };
  const glm::ivec2 actual = engine::grid::index_to_grid_position(idx, map_c.xmax, map_c.ymax);
  ASSERT_EQ(expected, actual);
};

TEST(TestSuite, MoveToOccupiedTile)
{
  // arrange
  entt::registry r;
  default_setup(r);
  const auto dungeon_e_0 = Factory_DataDungeonActor::create(r, {});
  add_entity_to_map(r, dungeon_e_0, 0);
  const auto dungeon_e_1 = Factory_DataDungeonActor::create(r, {});
  add_entity_to_map(r, dungeon_e_1, 1);

  // act
  const auto& map_c = get_first_component<MapComponent>(r);
  const auto next_idx = engine::grid::grid_position_to_index({ 1, 0 }, map_c.xmax);
  auto next_wp = engine::grid::index_to_world_position(next_idx, map_c.xmax, map_c.ymax, map_c.tilesize);
  next_wp += glm::vec2{ map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
  move_action_common(r, dungeon_e_0, next_wp);

  // assert: has not moved
  const auto it = std::find(map_c.map.begin(), map_c.map.end(), dungeon_e_0);
  const auto idx = static_cast<int>(it - map_c.map.begin());
  const glm::ivec2 expected = glm::ivec2{ 0, 0 }; // hasnt moved
  const glm::ivec2 actual = engine::grid::index_to_grid_position(idx, map_c.xmax, map_c.ymax);
  ASSERT_EQ(expected, actual);
};

TEST(TestSuite, DungeonEntityDropsScrap)
{
  // arrange
  entt::registry r;
  default_setup(r);

  const auto attacker_e = Factory_DataDungeonActor::create(r, {});
  r.emplace<AttackComponent>(attacker_e, 10); // comp could be on shotgun
  add_entity_to_map(r, attacker_e, 0);

  DataDungeonActor desc;
  desc.hp = 10;
  desc.max_hp = 10;
  const auto defender_e = Factory_DataDungeonActor::create(r, desc);
  add_entity_to_map(r, defender_e, 1);

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
  const auto& map_c = get_first_component<MapComponent>(r);
  auto& item_c = r.get<ItemTypeComponent>(map_c.map[1]);
  ASSERT_EQ(ItemType::scrap, item_c.type);
};

TEST(TestSuite, MoveToItemTileWithInventory)
{
  // arrange
  entt::registry r;
  default_setup(r);

  // dungeon entity with inventory...
  DataDungeonActor desc;
  desc.team = AvailableTeams::player;
  const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);
  r.emplace<DefaultBody>(dungeon_e, DefaultBody(r));
  r.emplace<DefaultInventory>(dungeon_e, DefaultInventory(r, 6 * 5));
  // r.emplace<InitBodyAndInventory>(dungeon_e); // not init with default stuff
  r.emplace<PlayerComponent>(dungeon_e);
  add_entity_to_map(r, dungeon_e, 0);

  // scrap item on other tile...
  const auto item_e = Factory_DataScrap::create(r, {});
  add_entity_to_map(r, item_e, 1);

  // act
  {
    const auto& map_c = get_first_component<MapComponent>(r);
    const auto next_idx = engine::grid::grid_position_to_index({ 1, 0 }, map_c.xmax);
    auto next_wp = engine::grid::index_to_world_position(next_idx, map_c.xmax, map_c.ymax, map_c.tilesize);
    next_wp += glm::vec2{ map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
    move_action_common(r, dungeon_e, next_wp);
  }

  // assert: dungeon entity has moved
  {
    const auto& map_c = get_first_component<MapComponent>(r);
    const auto it = std::find(map_c.map.begin(), map_c.map.end(), dungeon_e);
    const auto idx = static_cast<int>(it - map_c.map.begin());
    const glm::ivec2 expected = glm::ivec2{ 1, 0 };
    const glm::ivec2 actual = engine::grid::index_to_grid_position(idx, map_c.xmax, map_c.ymax);
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
    const auto& map_c = get_first_component<MapComponent>(r);
    const auto it = std::find(map_c.map.begin(), map_c.map.end(), item_e);
    ASSERT_TRUE(it == map_c.map.end());
  }
};

TEST(TestSuite, MoveToItemTileWithNoInventory)
{
  // arrange
  entt::registry r;
  default_setup(r);

  // dungeon entity with no inventory...
  DataDungeonActor desc;
  desc.team = AvailableTeams::player;
  const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);
  add_entity_to_map(r, dungeon_e, 0);

  // scrap item on other tile...
  const auto item_e = Factory_DataScrap::create(r, {});
  add_entity_to_map(r, item_e, 1);

  // act
  {
    const auto& map_c = get_first_component<MapComponent>(r);
    const auto next_idx = engine::grid::grid_position_to_index({ 1, 0 }, map_c.xmax);
    auto next_wp = engine::grid::index_to_world_position(next_idx, map_c.xmax, map_c.ymax, map_c.tilesize);
    next_wp += glm::vec2{ map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
    move_action_common(r, dungeon_e, next_wp);
  }

  // assert: dungeon entity has not moved
  {
    const auto& map_c = get_first_component<MapComponent>(r);
    const auto it = std::find(map_c.map.begin(), map_c.map.end(), dungeon_e);
    const auto idx = static_cast<int>(it - map_c.map.begin());
    const glm::ivec2 expected = glm::ivec2{ 0, 0 };
    const glm::ivec2 actual = engine::grid::index_to_grid_position(idx, map_c.xmax, map_c.ymax);
    ASSERT_EQ(expected, actual);
  }
};