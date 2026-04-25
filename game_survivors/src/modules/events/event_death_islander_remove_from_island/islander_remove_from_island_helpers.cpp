#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "islander_remove_from_island_helpers.hpp"
#include "modules/actors/actor_islander/islander_components.hpp"
#include "modules/actors/actor_islander/islander_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/actors/actor_rock/rock_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/events/event_island_to_boat/island_to_boat_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_helpers.hpp"

namespace game2d {

void
handle_death_event__islander_remove_from_island(entt::registry& r, const DeathEvent& evt)
{
  const auto dead_e = evt.dead;

  // only remove islanders
  if (!r.all_of<MovementIslandComponent>(dead_e))
    return;

  const auto tilesize = SINGLE_Islands::instance.tilesize;
  const auto pos = get_position(r, dead_e);
  const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
  const auto gp = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);
  const auto id = engine::encode_cantor_pairing_function(gp.x, gp.y);

  // figure out what island they were on.
  const auto island_e = SINGLE_Islands::instance.id_to_island_eid.at(id);
  auto& island_c = r.get<DebugContoursComponent>(island_e);

  // remove them from the island
  const auto it = std::find_if(island_c.occupied_island_xy.begin(),
                               island_c.occupied_island_xy.end(),
                               [&dead_e](const auto& other) { return other.second == dead_e; });

  if (it == island_c.occupied_island_xy.end()) {
    SDL_Log("Warning: something died, but cant find it on the island.");
    return;
  }

  SDL_Log("something died! removing %i %i from island %zu", gp.x, gp.y, (uint32_t)island_e);
  island_c.occupied_island_xy.erase(it);

  // What to do if that islander was a player's islander is now dead?
  // fro the moment, jsut return control to the boat
  auto* movement_c = r.try_get<MovementIslandComponent>(dead_e);
  if (movement_c && movement_c->boat_e != entt::null && r.valid(movement_c->boat_e)) {
    auto boat_e = movement_c->boat_e;
    r.remove<DroppedAnchorComponent>(boat_e);
    r.emplace<MovementDirectComponent>(boat_e);

    // send an event
    IslandToBoatEvent evt;
    auto& evts_c = SINGLE_Events::instance;
    evts_c.dispatcher->trigger(evt);
    evts_c.dispatcher->update();
  }

  // Check if the island is now clear of enemies
  int enemies_remaining = 0;
  for (const auto& [gp, e] : island_c.occupied_island_xy) {
    const auto& team_c = r.get<TeamComponent>(e);
    if (team_c.team == AvailableTeams::enemy) {
      enemies_remaining++;
    }
  }

  const auto unoccupied = get_unoccupied_tiles(island_c);
  if (enemies_remaining == 0 && !unoccupied.empty()) {
#if defined(_DEBUG)
    static engine::RandomState lighthouse_rnd(0); // same roll every time
#else
    static engine::RandomState lighthouse_rnd(engine::get_system_time_for_seed());
#endif

    SDL_Log("Island cleared of enemies");
    auto rnd_unoccipied_idx = engine::rand_det_s(lighthouse_rnd.rng, 0, (int)unoccupied.size());
    spawn_lighthouse(r, island_c, unoccupied[rnd_unoccipied_idx]);
    spawn_cannon(r, lighthouse_rnd, island_e);
    // auto islander_tag = "actor_islanddweller_common_person";
    // spawn_islander_unoccupied(r, lighthouse_rnd, island_e, islander_tag, AvailableTeams::player, true);

    // play some audio.
    //  static engine::RandomState audio_rnd(0);
    //  const int rnd_audio = engine::rand_det_s(audio_rnd.rng, 1, 7);
    //  create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "POSITIVE_0" + std::to_string(rnd_audio) });
  }
}

} // namespace game2d