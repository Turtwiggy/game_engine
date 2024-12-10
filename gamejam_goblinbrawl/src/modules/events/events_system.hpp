#pragma once

#include <entt/entt.hpp>

namespace game2d {

/* EXAMPLE...

struct DamageEvent
{
  entt::entity from;
  entt::entity to;
  int amount;
};
void handleDamageEvent(entt::registry& registry, const DamageEvent& event)...

entt::dispatcher dispatcher;
dispatcher.sink<DamageEvent>().connect<&handle_damage_event>();
dispatcher.trigger(damage_event);
dispatcher.update(); // dispatch events
*/

void
init_events_system(entt::registry& r);

void
update_events_system(entt::registry& r);

} // namespace game2d