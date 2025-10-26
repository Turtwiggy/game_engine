#include "pch.hpp"

#include "island_return_to_boat_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_islanddweller/islanddweller_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/events/event_island_to_boat/island_to_boat_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_return_to_boat_land_immunity/island_return_to_boat_land_immunity_components.hpp"
#include "modules/ui/ui_scene_survive_onboarding/ui_survive_onboarding_components.hpp"

namespace game2d {

void
update_island_return_to_boat_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  // note: if the onboarding screen is open, ignore this system.
  // GET_FIRST_OR_RETURN(SINGLE_InfoUI, r, onboarding_e, onboarding_c);
  // if (!onboarding_c.complete)
  //   return;

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  // allow players to press button to return to boat
  {
    const auto view = r.view<const MovementIslandComponent, const InputComponent>();
    for (const auto& [e, movement_c, input_c] : view.each()) {
      const bool return_to_boat_button = has(input_c.button_e, ActionStateEnum::DOWN);
      if (!return_to_boat_button)
        continue;
      r.emplace_or_replace<WantToReturnToBoat>(e);
    }
  }

  const auto view =
    r.view<const MovementIslandComponent, const TransformComponent, const InputComponent, const WantToReturnToBoat>();
  for (const auto& [e, movement_c, t_c, input_c, req_c] : view.each()) {

    // process event.
    r.remove<WantToReturnToBoat>(e);

    // remove the player.
    dead.dead.push_back(e);

    // set the tile as "unoccupied"
    const int tilesize = SINGLE_Islands::instance.tilesize;
    const auto pos = glm::vec2{ t_c.position.x, t_c.position.y };
    const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
    const auto gp = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);
    const auto id = engine::encode_cantor_pairing_function(gp.x, gp.y);
    const auto& all_islands_c = SINGLE_Islands::instance;
    const auto island_e = all_islands_c.id_to_island_eid.at(id);
    auto& island_c = r.get<DebugContoursComponent>(island_e);
    const auto it = std::find_if(island_c.occupied_island_xy.begin(),
                                 island_c.occupied_island_xy.end(),
                                 [&](const auto& other) { return other.second == e; });
    island_c.occupied_island_xy.erase(it);

    if (!r.valid(e))
      continue;

    // add back control to your boat.
    auto boat_e = movement_c.boat_e;

    // the boat has been destroyed
    if (!r.valid(boat_e))
      return;

    r.remove<DroppedAnchorComponent>(boat_e);
    r.emplace<MovementDirectComponent>(boat_e);
    r.emplace_or_replace<IslandCollisionImmunity>(boat_e, IslandCollisionImmunity{ .island_e = island_e });

    // give the boat a small push in the dir away from the island.
    const float impulse_amount = 1.0f;
    const auto boat_pos = get_position(r, boat_e);
    const auto islander_pos = pos;
    const auto raw_dir = boat_pos - islander_pos;
    const auto nrm_dir = engine::normalize_safe(raw_dir);
    const auto boat_body_id = r.get<PhysicsBodyComponent>(boat_e).bodyId;
    const auto boat_mass = b2Body_GetMass(boat_body_id);
    const auto impuse = boat_mass * impulse_amount;
    b2Body_ApplyLinearImpulseToCenter(boat_body_id, impuse * b2Vec2{ nrm_dir.x, nrm_dir.y }, true);

    // send an event
    IslandToBoatEvent evt;
    auto& evts_c = SINGLE_Events::instance;
    evts_c.dispatcher->trigger(evt);
    evts_c.dispatcher->update();
  }
}

} // namespace game2d