#include "pch.hpp"

#include "anchor_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_islander/islander_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/systems/system_island_revive/island_revive_components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_anchor_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  for (const auto& [e, anchor_c, revive_c] : r.view<DroppedAnchorComponent, const RevivableComponent>().each()) {
    if (anchor_c.target_e != entt::null)
      continue;

    // spawn target for the boat
    anchor_c.target_e = spawn(r, "actor_anchor");
    // give_life(r, anchor_c.target_e, get_position(r, e), { 16, 16 });

    // navigate to base island
    give_life(r, anchor_c.target_e, { 0, 0 }, { 16, 16 });

    ApplyForceToDynamicTarget tgt_c;
    tgt_c.orbit = true;
    tgt_c.reduce_thrusters = true;
    tgt_c.distance_to_reduce_thrust_meters = pixels_to_meters(default_map_tilesize * 10.0f);
    r.emplace<ApplyForceToDynamicTarget>(e, tgt_c);
    r.emplace<PhysicsDynamicTarget>(e, PhysicsDynamicTarget{ .target = anchor_c.target_e });

#if defined(_DEBUG)
    SDL_Log("current speed was: %f", r.get<ActorSpeedComponent>(e).current_speed);
#endif

    r.get<ActorSpeedComponent>(e).current_speed = 3.0f;
  }
}

} // namespace game2d