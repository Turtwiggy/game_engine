#include "pch.hpp"

#include "island_stranded_islander_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "entt/entity/entity.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_revive/island_revive_components.hpp"

namespace game2d {

void
update_island_stranded_islander_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto view = r.view<const MovementIslandComponent, const TransformComponent, const PlayerComponent>();
  for (const auto& [e, movement_c, t_c, player_c] : view.each()) {

    bool boat_is_dead = r.try_get<RevivableComponent>(movement_c.boat_e) != nullptr;
    bool you_are_revive_islander = r.try_get<RevivableComponent>(e);

    if (!boat_is_dead) // boat is alive: cant be stranded
      continue;

    if (you_are_revive_islander) // if revive islander, dont delete it
      continue;

    // You're stranded!
    SDL_Log("Boat died; you were on an island");

    // remove the player.
    dead.dead.push_back(e);

    // remove from island you died island
    const auto island_e = movement_c.island_e;

    // set the tile as "unoccupied"
    auto& island_c = r.get<DebugContoursComponent>(island_e);
    const auto& all_islands_c = SINGLE_Islands::instance;
    const auto it = std::find_if(island_c.occupied_island_xy.begin(),
                                 island_c.occupied_island_xy.end(),
                                 [&](const auto& other) { return other.second == e; });
    if (it == island_c.occupied_island_xy.end()) {
      SDL_Log("Warning: some stranded islander existed, but cant find it on the island.");
      continue;
    }
    island_c.occupied_island_xy.erase(it);
  }
}

} // namespace game2d