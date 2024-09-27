#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/physics/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/map/components.hpp"
#include "modules/spaceship_designer/generation/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"

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

  const auto dungeon_e = get_first<DungeonGenerationResults>(r);
  if (dungeon_e == entt::null)
    return;
  const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

  const auto& view = r.view<const PlayerComponent, const MovementJetpackComponent>();
  for (const auto& [e, req_c, jetpack_c] : view.each()) {
    const auto pos = get_position(r, e);
    const auto gp = engine::grid::worldspace_to_grid_space(pos, map_c.tilesize);

    // check if outside ship
    const auto rooms = inside_room(r, gp);
    const bool in_room = rooms.size() > 0;
    if (!in_room)
      continue;

    const auto worldspace_pos = get_position(r, e);
    const glm::vec2 worldspace_clamped = engine::grid::worldspace_to_clamped_world_space(worldspace_pos, map_c.tilesize);
    const glm::vec2 offset = { map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
    const auto final_pos = worldspace_clamped + offset;

    r.remove<MovementJetpackComponent>(e);

    // change from dynamic to kinematic
    fmt::println("(jetpack => dungeon) setting bodytype to kinematic");
    r.get<PhysicsBodyComponent>(e).body->SetType(b2_kinematicBody);
    set_position(r, e, final_pos);

    // change camera
    remove_if_exists<CameraFollow>(r, e);
    r.emplace<CameraLerpToTarget>(e);
    auto camera_e = get_first<OrthographicCamera>(r);
    set_position(r, camera_e, final_pos);

    // create_empty<RequestUpdateFOV>(r);
  }
}

} // namespace game2d