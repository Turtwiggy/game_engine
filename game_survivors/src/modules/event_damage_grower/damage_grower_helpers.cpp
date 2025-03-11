#include "pch.hpp"

#include "damage_grower_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actor_enemy_grower/enemy_grower_components.hpp"
#include "modules/event_damage/event_damage_components.hpp"
#include "modules/event_death/components.hpp"
#include "modules/events/events_components.hpp"

namespace game2d {

void
handle_damage_event__grower(entt::registry& r, const DamageEvent& evt)
{
  // note: evt.to is a fixture.

  if (evt.to == entt::null)
    return;

  const auto parent_e = r.get<HasParentComponent>(evt.to).parent;
  auto* grower_c = r.try_get<GrowerComponent>(parent_e);
  if (!grower_c)
    return; // not a grower

  const float grower_min_size = 32;

  // shrink! you took damage.
  grower_c->radius_pixels -= 25;
  grower_c->radius_pixels = glm::max(grower_c->radius_pixels, grower_min_size);

  if (grower_c->radius_pixels <= grower_min_size) {

    // die, die, die!
    auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
    dead.dead.emplace(parent_e);

    // Send death event.
    DeathEvent d_evt;
    d_evt.killed_by = evt.from; // can be entt::null
    d_evt.dead = parent_e;      // parent not fixture
    auto& evts = get_first_component<SINGLE_Events>(r);
    evts.dispatcher->trigger(d_evt);
    evts.dispatcher->update();
  }
};

} // namespace game2d