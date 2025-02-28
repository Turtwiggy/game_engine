#include "event_coll_player_vacuum_orb_helpers.hpp"

#include "event_coll_player_vacuum_orb_components.hpp"

#include "engine/lifecycle/components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"

namespace game2d {

void
handle_player_enter_vacuum_orb(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [zone_e, item_e] = coll<XpZoneComponent, ItemVacuumOrbComponent>(r, evt.a, evt.b);
  if (zone_e == entt::null || item_e == entt::null)
    return;

  SDL_Log("You collided with a vacuum orb");

  // Remove all xp items from the physics system.
  // Make them all fly to the player.
  // Give that amount of xp if the distance is < than some amount to the player

  // auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  // dead.dead.emplace(item_e);
}

} // namespace game2d