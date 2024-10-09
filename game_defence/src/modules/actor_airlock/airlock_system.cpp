#include "airlock_system.hpp"

#include "actors/actor_helpers.hpp"
#include "components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "modules/actor_door/components.hpp"
#include "modules/actor_door/door_helpers.hpp"
#include "modules/map/components.hpp"
#include "modules/spaceship_designer/spaceship_designer_helpers.hpp"
#include "modules/system_cooldown/components.hpp"

namespace game2d {
using namespace engine::grid;

// Either:
// If on the north or south side, show a button "enter" that will allow you in the airlock
// If it's not full and you click it, go in then airlock, and after X seconds, you're on the other side

// Or:
// If you're on the side of the airlock and nothings in it, open that door
// If you're on the side of the airlock but it's full, doors are closed

void
update_airlock_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  for (const auto& [e, airlock_c] : r.view<AirlockComponent>().each()) {
    //
    // What gridpos is the airlock?
    const auto wp = get_position(r, e);
    const auto gp = worldspace_to_grid_space(wp, map_c.tilesize);
    const auto idx = grid_position_to_index(gp, map_c.xmax);

    const auto mobs_airlock = contains_mobs(r, gp);
    const auto mobs_in_airlock = mobs_airlock.size() > 0;
    const auto one_mob_in_airlock = mobs_airlock.size() == 1;

    const auto door_north = airlock_c.door_north;
    const auto door_south = airlock_c.door_south;

    // Something entered the airlock...
    //
    if (one_mob_in_airlock && airlock_c.mob_in_airlock == entt::null) {
      airlock_c.mob_in_airlock = mobs_airlock[0];

      // how long to spend in airlock
      CooldownComponent cooldown;
      cooldown.time_max = 3.0f;
      cooldown.time = cooldown.time_max;
      const auto& cooldown_c = r.emplace<CooldownComponent>(e, cooldown);

      fmt::println("mob entered airlock... shutting doors");

      // shut the doors
      auto* e_north = r.try_get<Edge>(door_north);
      auto* e_south = r.try_get<Edge>(door_south);
      if (!e_north) {
        const auto& door_c = r.get<DoorComponent>(door_north);
        r.emplace<Edge>(door_north, door_c.edge_copy);
        instantiate_edge(r, door_north, map_c);
        r.remove<DoorComponent>(door_north); // disable doors
      }
      if (!e_south) {
        const auto& door_c = r.get<DoorComponent>(door_south);
        r.emplace<Edge>(door_south, door_c.edge_copy);
        instantiate_edge(r, door_south, map_c);
        r.remove<DoorComponent>(door_south); // disable doors
      }
    }

    // Something left the airlock...
    //
    if (!mobs_in_airlock && airlock_c.mob_in_airlock != entt::null) {
      airlock_c.mob_in_airlock = entt::null;
      fmt::println("mob exited airlock...");
    }

    // If the airlock is full...
    //
    if (airlock_c.mob_in_airlock != entt::null) {

      // ... and the airlock is ready to release...
      //
      auto* cooldown_c = r.try_get<CooldownComponent>(e);
      if (cooldown_c && cooldown_c->time <= 0.0f) {
        fmt::println("airlock releasing...");
        r.remove<CooldownComponent>(e);

        // allow doors to open
        r.emplace<DoorComponent>(airlock_c.door_north, DoorComponent{ airlock_c.north_edge_copy });
        r.emplace<DoorComponent>(airlock_c.door_south, DoorComponent{ airlock_c.south_edge_copy });
      }
    }
  }

  //
}

} // namespace game2d