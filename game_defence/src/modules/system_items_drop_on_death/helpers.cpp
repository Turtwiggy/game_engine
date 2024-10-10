#include "helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/map/components.hpp"
#include "modules/map/helpers.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

namespace game2d {

//
// something should drop their inventory
//
void
drop_inventory_on_death_callback(entt::registry& r, const entt::entity e)
{
  auto& map = get_first_component<MapComponent>(r);

  // it's possible you dont exist if you died off-grid i.e. a space entity off-map
  const auto info_opt = get_entity_mapinfo(r, e);
  if (info_opt.has_value()) {
    const auto info = info_opt.value();
    const auto idx = info.idx_in_map;
    const auto pos = engine::grid::index_to_world_position_center(idx, map.xmax, map.ymax, map.tilesize);
    remove_entity_from_map(r, info);
  }

  const auto inv = r.get<DefaultInventory>(e); // take a copy
  r.remove<DefaultInventory>(e);               // remove inv from dead unit

  // add inv to floor
  auto item_e = spawn_item(r, "lootbag");
  set_position(r, item_e, get_position(r, e));

  // replace the lootbag inventory with the dead player's inventory
  r.replace<DefaultInventory>(item_e, inv);
};

} // namespace game2d