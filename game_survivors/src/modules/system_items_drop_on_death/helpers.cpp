#include "helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/map/components.hpp"
#include "engine/map/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_player_xp_coll/event_player_xp_coll_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

namespace game2d {

// something should drop their inventory
//
void
drop_inventory_on_death_callback(entt::registry& r, const entt::entity e)
{
  GET_FIRST_OR_RETURN(MapComponent, r, map_e, map_c);

  // it's possible you dont exist if you died off-grid i.e. a space entity off-map
  const auto info_opt = get_entity_mapinfo(r, e);
  if (info_opt.has_value()) {
    const auto info = info_opt.value();
    const auto idx = info.idx_in_map;
    const auto pos = engine::grid::index_to_world_position_center(idx, map_c.xmax, map_c.ymax, map_c.tilesize);
    remove_entity_from_map(r, info);
  }

  const auto inv = r.get<DefaultInventory>(e); // take a copy
  r.remove<DefaultInventory>(e);               // remove inv from dead unit

  // add inv to floor
  auto item_e = spawn(r, "lootbag");
  give_life(r, item_e, get_position(r, e), { 16, 16 });

  // replace the lootbag inventory with the dead player's inventory
  r.replace<DefaultInventory>(item_e, inv);
};

void
drop_xp_on_death_callback(entt::registry& r, const entt::entity e)
{
  const auto item_e = spawn(r, "xp");
  give_life(r, item_e, get_position(r, e), { 16, 16 });
  r.emplace<TeamComponent>(item_e, AvailableTeams::neutral);
  r.emplace<XpComponent>(item_e);
  r.remove<OnDeathCallback>(item_e); // xp doesnt do anything on it's death?
};

} // namespace game2d