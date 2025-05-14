#include "death_throes_system.hpp"

#include "death_throes_components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_components.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/events/event_death/components.hpp"
#include "modules/events/events_core/events_components.hpp"

namespace game2d {

void
update_death_throes_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  auto view = r.view<DeathThroesComponent, const IsDyingComponent>();
  for (const auto& [e, death_c, dying_c] : view.each()) {
    death_c.death_throws_time_cur -= dt;

    // set init death throes state
    if (death_c.death_throws_time_cur == death_c.death_throws_time_max) {
      const auto red = engine::SRGBColour({ 1.0f, 0.0f, 0.0f, 1.0f });
      set_colour(r, e, red);
      r.get<DefaultColour>(e).colour = red;
    }

    // Keep flashing & scaling to indicate you're dying
    auto* scale_c = r.try_get<RequestHitScaleComponent>(e);
    if (!scale_c)
      r.emplace<RequestHitScaleComponent>(e);

    // check if you're actually dead.
    if (death_c.death_throws_time_cur > 0.0f)
      continue; // continue dying

    // Send death event.
    DeathEvent d_evt;
    d_evt.killed_by = death_c.evt_from;
    d_evt.dead = e; // parent not fixture
    auto& evts = get_first_component<SINGLE_Events>(r);
    evts.dispatcher->trigger(d_evt);
    evts.dispatcher->update();

    SDL_Log("Completed death throes");
    dead.dead.push_back(e);
  }
}

} // namespace game2d