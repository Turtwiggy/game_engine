#include "create_item_system.hpp"

#include "create_item_components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/events/event_coll_player_gold/event_coll_player_gold_components.hpp"
#include "modules/events/event_coll_player_hp/event_coll_player_hp_components.hpp"
#include "modules/events/event_coll_player_sea_mine/event_coll_player_sea_mine_components.hpp"
#include "modules/events/event_coll_player_vacuum_orb/event_coll_player_vacuum_orb_components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_create_item_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto view = r.view<CreateItemRequest>();
  for (const auto& [e, req_c] : view.each()) {

    auto item_e = spawn(r, req_c.item);
    give_life(r, item_e, req_c.position, { default_map_unit_tilesize, default_map_unit_tilesize });
    r.emplace<TeamComponent>(item_e, AvailableTeams::neutral);
    r.remove<OnDeathCallbacks>(item_e);

    const auto fixture_e = get_fixture_by_tag(r, item_e, "fixture_item");
    if (req_c.item == "item_gold")
      r.emplace<ItemGoldComponent>(fixture_e);
    else if (req_c.item == "item_hp_pack")
      r.emplace<ItemHealingPackComponent>(fixture_e);
    else if (req_c.item == "item_sea_mine")
      r.emplace<ItemSeaMineComponent>(fixture_e);
    else if (req_c.item == "item_vacuum_orb")
      r.emplace<ItemVacuumOrbComponent>(fixture_e);
    else
      throw std::runtime_error("Unknown item type");
  }
  r.destroy(view.begin(), view.end());
}

} // namespace game2d