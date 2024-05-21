#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/ui_spaceship_designer/components.hpp"
#include "renderer/transform.hpp"

#include <iostream>

namespace game2d {

void
update_gen_dungeons_system(entt::registry& r)
{
  const auto& requests = r.view<RequestGenerateDungeonComponent>();
  if (requests.size() == 0)
    return;
  r.destroy(requests.begin(), requests.end()); // process request

  auto& map = get_first_component<MapComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // clear old map
  {
    // Mark all rooms as ded, and remove room components.
    const auto& wall_view = r.view<Wall>();
    for (const auto& [wall_e, wall_c] : wall_view.each()) {
      dead.dead.emplace(wall_e);
      r.remove<Room>(wall_e);
    }
    const auto& room_view = r.view<Room>();
    for (const auto& [room_e, room_c] : room_view.each()) {
      dead.dead.emplace(room_e);
      r.remove<Room>(room_e);
    }
    const auto& view = r.view<PathfindComponent, TransformComponent>();
    for (const auto& [e, pc, t] : view.each()) {
      const auto global_grid_pos = engine::grid::world_space_to_grid_space({ t.position.x, t.position.y }, map.tilesize);
      const auto global_idx = engine::grid::grid_position_to_index(global_grid_pos, map.xmax);
      map.map[global_idx].clear();

      dead.dead.emplace(e);
      r.emplace<WaitForInitComponent>(e);
    }
  }

  static int seed = 0;
  seed++; // Increase seed everytime a map is generated
  engine::RandomState rnd(seed);

  DungeonGenerationCriteria dungeon_parameters;
  dungeon_parameters.max_rooms = 50;
  dungeon_parameters.room_size_min = glm::max(dungeon_parameters.room_size_min, 0);
  dungeon_parameters.room_size_max = glm::min(dungeon_parameters.room_size_max, map.xmax);

  const DungeonGenerationResults dungeon_result = create_all_rooms(dungeon_parameters, map, rnd);
  instantiate_walls(r, dungeon_result);

  // Steps after initial initialization...
  set_generated_entity_positions(r, dungeon_result, rnd);
  set_player_positions(r, dungeon_result, rnd);

  // set exit door position
  // bug: exit placed on generated entities. not so bad?
  // Room& room = d.rooms[d.rooms.size() - 1];
  // glm::ivec2 middle = room_center(room);
  // create_dungeon_entity(editor, game, EntityType::tile_type_exit, middle);

  // Set the cost for all of the tiles
  for (int i = 0; i < dungeon_result.wall_or_floors.size(); i++) {
    // const auto grid_xy = engine::grid::index_to_grid_position(i, map.xmax, map.ymax);
    const auto empty_e = create_gameplay(r, EntityType::empty_no_transform);
    map.map[i].push_back(empty_e);
    auto& pathfinding_c = r.emplace<PathfindComponent>(empty_e);
    if (dungeon_result.wall_or_floors[i] == 1)
      pathfinding_c.cost = -1; // impassable wall
    else
      pathfinding_c.cost = 0; // floor
  }

  std::cout << "dungeon generated" << std::endl;
}

} // namespace game2d