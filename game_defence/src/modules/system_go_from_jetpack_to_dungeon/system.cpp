#include "system.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/system_fov/components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"

#include <box2d/b2_math.h>

namespace game2d {

void
update_go_from_jetpack_to_dungeon_system(entt::registry& r)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto dungeon_e = get_first<DungeonGenerationResults>(r);
  if (dungeon_e == entt::null)
    return;
  const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

  const auto& view = r.view<const PlayerComponent, const MovementJetpackComponent>();
  for (const auto& [e, req_c, jetpack_c] : view.each()) {
    const auto pos = get_position(r, e);
    const auto gp = engine::grid::worldspace_to_grid_space(pos, map.tilesize);

    // check if outside ship
    const auto rooms = inside_room(r, gp);
    const bool in_room = rooms.size() > 0;
    if (!in_room)
      continue;

    const auto worldspace_pos = get_position(r, e);
    const glm::vec2 worldspace_clamped = engine::grid::worldspace_to_clamped_world_space(worldspace_pos, map.tilesize);
    const glm::vec2 offset = { map.tilesize / 2.0f, map.tilesize / 2.0f };
    const auto final_pos = worldspace_clamped + offset;

    remove_components(r, e, DataJetpackActor{});

    DataDungeonActor player_desc;
    player_desc.team = AvailableTeams::player;
    player_desc.hp = 100;
    player_desc.max_hp = 100;
    player_desc.pos = final_pos;
    add_components(r, e, player_desc);

    // change camera
    remove_if_exists<CameraFollow>(r, e);
    r.emplace<CameraLerpToTarget>(e);
    auto camera_e = get_first<OrthographicCamera>(r);
    set_position(r, camera_e, player_desc.pos);

    fmt::println("going from jetpack to dungeon character...");
    create_empty<RequestUpdateFOV>(r);
  }
}

} // namespace game2d