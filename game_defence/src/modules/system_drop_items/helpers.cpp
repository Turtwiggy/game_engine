#include "helpers.hpp"

#include "actors/actors.hpp"
#include "actors/bags/items.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"

#include <algorithm>

namespace game2d {

//
// Really, something should drop their inventory
// for the moment, just drop a piece of scrap
//
void
drop_items_on_death_callback(entt::registry& r, const entt::entity e)
{
  auto& map = get_first_component<MapComponent>(r);

  // you SHOULD exist when you die. you've not been removed yet.
  const auto it = std::find(map.map.begin(), map.map.end(), e);

  if (it == map.map.end()) {
    fmt::println("error: entity not in map.");
    exit(1); // crash
  }

  const int idx = static_cast<int>(it - map.map.begin());
  auto pos = engine::grid::index_to_world_position_center(idx, map.xmax, map.ymax, map.tilesize);

  DataScrap desc;
  desc.pos = pos;
  desc.sprite = "SCRAP";
  const auto scrap_e = Factory_DataScrap::create(r, desc);

  // Replace yourself with a newly spawned scrap item
  map.map[idx] = entt::null;
  add_entity_to_map(r, scrap_e, idx);

  fmt::println("item added to mapidx: {}", idx);
};

} // namespace game2d