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
  const auto& evts_c = get_first_component<SINGLE_Events>(r);
  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);

  gold_c.temp_amount += 5;
  SDL_Log("You collided with gold.. new gold: %i", gold_c.temp_amount);

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.push_back(item_e);
}

} // namespace game2d