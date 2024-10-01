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
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include <box2d/b2_body.h>

namespace game2d {

void
update_go_from_dungeon_to_jetpack_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  auto& map_c = r.get<MapComponent>(map_e);

  const auto dungeon_e = get_first<DungeonGenerationResults>(r);
  if (dungeon_e == entt::null)
    return;
  const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

  const auto& view = r.view<const PlayerComponent>(entt::exclude<MovementJetpackComponent>);
  for (const auto& [e, player_e] : view.each()) {
    const auto pos = get_position(r, e);
    const auto gp = engine::grid::worldspace_to_grid_space(pos, map_c.tilesize);

    const auto rooms = inside_room(r, gp);
    const bool in_room = rooms.size() > 0;
    if (in_room)
      continue; // should be: outside ship

    // Remove from the map_c when leaving the dungeon
    const auto& lerp_info = r.get<LerpToFixedTarget>(e);
    int to_idx = engine::grid::worldspace_to_index(lerp_info.b, map_c.tilesize, map_c.xmax, map_c.ymax);
    fmt::println("wants to leave (dungeon) at {}", to_idx);
    auto& map_es = map_c.map[to_idx];
    const auto to_remove = [e](const entt::entity& other_e) { return other_e == e; };
    map_es.erase(std::remove_if(map_es.begin(), map_es.end(), to_remove), map_es.end());
    r.remove<LerpToFixedTarget>(e);

    r.emplace<MovementJetpackComponent>(e);

    // change from kinematic to dynamic
    fmt::println("(dungeon => jetpack) setting bodytype to dynamic");
    r.get<PhysicsBodyComponent>(e).body->SetType(b2_dynamicBody);

    const auto final_pos = engine::grid::grid_space_to_world_space_center(gp, map_c.tilesize);
    set_position(r, e, final_pos);

    // change camera
    // remove_if_exists<CameraLerpToTarget>(r, e);
    // r.emplace<CameraFollow>(e);
    auto camera_e = get_first<OrthographicCamera>(r);
    set_position(r, camera_e, final_pos);

    //
  }
}

} // namespace game2d