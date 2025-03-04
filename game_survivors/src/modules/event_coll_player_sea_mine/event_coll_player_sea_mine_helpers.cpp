#include "event_coll_player_sea_mine_helpers.hpp"

#include "event_coll_player_sea_mine_components.hpp"

#include "engine/lifecycle/components.hpp"
#include "modules/actor_exploder/actor_exploder_helpers.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"

namespace game2d {

void
handle_player_enter_sea_mine(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [zone_e, item_e] = coll<XpZoneComponent, ItemSeaMineComponent>(r, evt.a, evt.b);
  if (zone_e == entt::null || item_e == entt::null)
    return;

  SDL_Log("You collided with a sea mine... exploding");

  //
  // use callbacks when need to update physics things,
  // because currently collision via physics world
  // and updating things during physics step makes physics sad
  //

  // Same as exploder logic...
  const auto item_par_e = r.get<HasParentComponent>(item_e).parent;
  add_explode_on_death_callback(r, item_par_e);

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.emplace(item_par_e);
}

} // namespace game2d