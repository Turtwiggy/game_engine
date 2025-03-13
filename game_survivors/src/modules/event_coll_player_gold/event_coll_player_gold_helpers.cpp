#include "event_coll_player_gold_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "event_coll_player_gold_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/system_item_gold/gold_components.hpp"

namespace game2d {

void
handle_player_enter_gold(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [zone_e, item_e] = coll<XpZoneComponent, ItemGoldComponent>(r, evt.a, evt.b);
  if (zone_e == entt::null || item_e == entt::null)
    return;
  const auto& evts_c = get_first_component<SINGLE_Events>(r);
  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);

  gold_c.temp_amount += 5;
  SDL_Log("You collided with gold.. new gold: %i", gold_c.temp_amount);

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.emplace(item_e);
}

} // namespace game2d