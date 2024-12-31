#include "combat_bleed_helpers.hpp"

#include "combat_bleed_components.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/events/events_components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_action_bar/ui_action_bar_components.hpp"

namespace game2d {

void
handle_end_turn_bleed(entt::registry& r, const EndTurnEvent& evt)
{
  if (evt.e == entt::null)
    return;

  auto* bleed_c = r.try_get<BleedComponent>(evt.e);
  if (!bleed_c)
    return;

  const auto& name_c = r.get<NameComponent>(evt.e);
  SDL_Log("%s takes damage from bleed.", name_c.name.c_str());

  // decrement counter
  bleed_c->turns_left -= 1;

  if (bleed_c->turns_left <= 0) {
    SDL_Log("Bleed done... removing...");
    r.remove<BleedComponent>(evt.e);
  };

  // Take bleed damage
  DamageEvent dmg_evt;
  dmg_evt.from = entt::null; // system
  dmg_evt.to = evt.e;
  dmg_evt.type = DamageType::PURE;
  dmg_evt.amount = 5; // bleed amount
  SDL_Log("Sending bleed event");

  const auto& evts = get_first_component<SINGLE_Events>(r);
  evts.dispatcher->trigger(dmg_evt);
  evts.dispatcher->update();
};

} // namespace game2d