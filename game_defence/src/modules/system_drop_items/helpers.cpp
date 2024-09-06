#include "helpers.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/ui_inventory/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

//
// something should drop their inventory
//
void
drop_inventory_on_death_callback(entt::registry& r, const entt::entity e)
{
  auto& map = get_first_component<MapComponent>(r);

  // you SHOULD exist when you die. you've not been removed. yet.
  const auto info_opt = get_entity_mapinfo(r, e);
  const auto info = info_opt.value();

  const auto idx = info.idx_in_map;
  const auto pos = engine::grid::index_to_world_position_center(idx, map.xmax, map.ymax, map.tilesize);

  const auto inv = r.get<DefaultInventory>(e); // take a copy
  r.remove<DefaultInventory>(e);               // remove inv from dead unit

  // clear the dead entity from the map
  remove_entity_from_map(r, info);

  // add inv to floor
  DataDungeonLootbag loot_desc;
  loot_desc.pos = pos;
  loot_desc.inventory = inv;
  Factory_DataDungeonLootbag::create(r, loot_desc);

  fmt::println("item add to map_idx: {} pos: {}", info.idx_in_map, info.idx_in_map_tile);
};

} // namespace game2d