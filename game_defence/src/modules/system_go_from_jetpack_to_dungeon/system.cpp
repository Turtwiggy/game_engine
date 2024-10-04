#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/physics/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/map/components.hpp"
#include "modules/map/helpers.hpp"
#include "modules/spaceship_designer/generation/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"

#include <box2d/b2_body.h>
#include <box2d/b2_math.h>

namespace game2d {

void
update_go_from_jetpack_to_dungeon_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);

  const auto& view = r.view<const PlayerComponent, const MovementJetpackComponent>();
  for (const auto& [e, req_c, jetpack_c] : view.each()) {
    const auto pos = get_position(r, e);
    const auto gp = engine::grid::worldspace_to_grid_space(pos, map_c.tilesize);

    const auto rooms = inside_room(r, gp);
    const bool in_room = rooms.size() > 0;
    if (!in_room)
      continue; // should be: inside ship

    r.remove<MovementJetpackComponent>(e);

    // Add entity to map_c when entering the dungeon
    const auto idx = engine::grid::grid_position_to_index(gp, map_c.xmax);
    fmt::println("wants to join (dungeon) at {}", idx);
    add_entity_to_map(r, e, idx);

    // change from dynamic to kinematic
    fmt::println("(jetpack => dungeon) setting bodytype to kinematic");
    r.get<PhysicsBodyComponent>(e).body->SetType(b2_kinematicBody);
    r.get<PhysicsBodyComponent>(e).body->SetLinearVelocity({ 0.0f, 0.0f });
    r.get<PhysicsBodyComponent>(e).body->SetAngularVelocity(0.0f);

    const auto final_pos = engine::grid::grid_space_to_world_space_center(gp, map_c.tilesize);
    set_position(r, e, final_pos);

    // create_empty<RequestUpdateFOV>(r);
  }
}

} // namespace game2d