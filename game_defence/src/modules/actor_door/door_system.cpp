#include "door_system.hpp"

#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "modules/actor_door/components.hpp"
#include "modules/actor_door/door_helpers.hpp"
#include "modules/spaceship_designer/spaceship_designer_helpers.hpp"

namespace game2d {

//
// After this update,
// everything that has a DoorComponent attached,
// will have it's edges updated to reflect if it has mobs next to it.
//

void
update_door_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);

  // open and close doors...
  //
  for (const auto& [e, door_c] : r.view<const DoorComponent>().each()) {
    const auto mobs_a = contains_mobs(r, door_c.edge_copy.a).size() > 0;
    const auto mobs_b = contains_mobs(r, door_c.edge_copy.b).size() > 0;
    const bool has_mobs = mobs_a || mobs_b;

    // if something is in front of the door, open the door
    //
    if (has_mobs) {
      if (auto* t_c = r.try_get<TransformComponent>(e)) {
        // fmt::println("mobs... opening door");
        r.remove<Edge>(e); // allow pathfinding
        r.remove<TransformComponent>(e);
        r.remove<SpriteComponent>(e);
        auto& physics = get_first_component<SINGLE_Physics>(r);
        const auto& p_c = r.get<PhysicsBodyComponent>(e);
        physics.world->DestroyBody(p_c.body);
        r.remove<PhysicsBodyComponent>(e);
      }
    }
    // if nothing is on either side of the door, shut the door
    //
    else {
      const auto* t_c = r.try_get<TransformComponent>(e);
      if (!t_c) {
        // fmt::println("no mobs... shutting door");
        r.emplace<Edge>(e, door_c.edge_copy);
        instantiate_edge(r, e, map_c);
      }
    }
  }
}

} // namespace game2d