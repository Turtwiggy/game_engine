#include "pch.hpp"

#include "event_coll_player_gold_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "event_coll_player_gold_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"

namespace game2d {

void
handle_player_enter_gold(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [pfixture_e, item_e] = coll<PlayerFixtureComponent, ItemGoldComponent>(r, evt.a, evt.b);
  if (pfixture_e == entt::null || item_e == entt::null)
    return;
  auto& evts_c = SINGLE_Events::instance;
  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);

  gold_c.temp_amount_pickup += 5;
  SDL_Log("You collided with gold.. new gold: %i", gold_c.temp_amount_pickup);

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.push_back(item_e);

  // {
  //   RequestToSpawnParticles request;
  //   request.key = "vfx_levelup_outer";
  //   request.position = get_position(r, par_e);
  //   create_empty<RequestToSpawnParticles>(r, request);
  // }
}

} // namespace game2d