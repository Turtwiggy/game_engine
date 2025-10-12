#include "pch.hpp"

#include "bump_damage_event_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_components.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"

namespace game2d {

void
handle_bump_event__damage(entt::registry& r, const BumpEvent& evt)
{
  const auto n_e = evt.to;
  const auto& team_c = r.get<const TeamComponent>(evt.from);

  // make the neighbour flash.
  r.emplace_or_replace<RequestHitScaleComponent>(n_e);

  if (auto* children_c = r.try_get<HasChildrenComponent>(n_e)) {
    for (const auto child_e : children_c->children)
      r.emplace_or_replace<RequestHitScaleComponent>(child_e);
  }

  if (const auto* hp_c = r.try_get<const HealthComponent>(n_e)) {

    // dont damage friendly-team things
    const auto& neighbour_team_c = r.get<const TeamComponent>(n_e);
    const bool same_team = neighbour_team_c.team == team_c.team;
    if (same_team) {
      // SDL_Log("A player collided with a friendly entity");
      return;
    }

    // note: this is a grid-based damage system with no fixtures.
    const DamageEvent evt{
      .from = entt::null,
      .to_parent = n_e,
      .to_fixture = n_e, // same as parent, as no fixture
      .amount = 1,
      .type = WEAPON_DAMAGE::KINETIC,
    };
    const auto& evts_c = SINGLE_Events::instance;
    evts_c.dispatcher->trigger(evt);
    evts_c.dispatcher->update();
  }
}

} // namespace game2d