#include "event_coll_player_gold_helpers.hpp"

#include "engine/lifecycle/components.hpp"
#include "event_coll_player_gold_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"

namespace game2d {

void
handle_player_enter_gold(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [zone_e, item_e] = coll<XpZoneComponent, ItemGoldComponent>(r, evt.a, evt.b);
  if (zone_e == entt::null || item_e == entt::null)
    return;

  SDL_Log("You collided with gold");

  // auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  // dead.dead.emplace(item_e);
}

} // namespace game2d