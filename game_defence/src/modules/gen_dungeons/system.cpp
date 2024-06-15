#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "helpers/line.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/gen_dungeons//helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/gen_dungeons/helpers/gen_players.hpp"
#include "modules/grid/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_combat_turnbased/components.hpp"

#include "imgui.h"

#include <SDL_scancode.h>
#include <iostream>
#include <vector>

namespace game2d {

void
update_gen_dungeons_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  auto& map = get_first_component<MapComponent>(r);
  auto& input = get_first_component<SINGLETON_InputComponent>(r);

  ImGui::Begin("Debug__Tunnels");
  for (const auto& [e, t_c] : r.view<Tunnel>().each()) {
    ImGui::Separator();

    ImGui::Text("TunnelLine0");
    for (const auto& t : t_c.line_0)
      ImGui::Text("%i %i", t.first, t.second);

    ImGui::Text("TunnelLine1");
    for (const auto& t : t_c.line_1)
      ImGui::Text("%i %i", t.first, t.second);
  }
  ImGui::End();

  ImGui::Begin("DebugDungeonGen");
  static int i = 0;
  if (get_key_down(input, SDL_SCANCODE_O))
    i--;
  if (get_key_down(input, SDL_SCANCODE_P))
    i++;
  ImGui::Text("i: %i", i);

  std::vector<Room> rooms;
  for (const auto& [e, room] : r.view<Room>().each())
    rooms.push_back(room);

  std::vector<Tunnel> tunnels;
  for (const auto& [e, t] : r.view<Tunnel>().each())
    tunnels.push_back(t);

  const auto grid_pos = engine::grid::world_space_to_grid_space(mouse_pos, map.tilesize);
  ImGui::Text("grid_pos: %i %i", grid_pos.x, grid_pos.y);

  const auto in_room = inside_room(map, rooms, grid_pos);
  ImGui::Text("Inside Room: %i", in_room);

  const auto in_tunnel = inside_tunnel(tunnels, grid_pos);
  ImGui::Text("Inside Tunnel: %i", in_tunnel);

  auto dungeon_e = get_first<DungeonGenerationResults>(r);
  if (dungeon_e != entt::null) {
    const auto dungeon_results = get_first_component<DungeonGenerationResults>(r);
    ImGui::Text("Have dungeon gen results");

    auto mouse_idx = engine::grid::grid_position_to_index(grid_pos, map.xmax);
    mouse_idx = glm::clamp(mouse_idx, 0, map.xmax * map.ymax - 1);

    ImGui::Text("mouse: %i is_wall_or_floor: %i", mouse_idx, dungeon_results.wall_or_floors[mouse_idx]);
  }

  ImGui::End();

  // HACK: force regenerate dungeon
  if (get_key_down(input, SDL_SCANCODE_I)) {
    const auto e = create_empty<RequestGenerateDungeonComponent>(r);
    r.emplace<RequestGenerateDungeonComponent>(e);
  }

  const auto& requests = r.view<RequestGenerateDungeonComponent>();
  if (requests.size() == 0)
    return;
  r.destroy(requests.begin(), requests.end()); // process request

  move_to_scene_start(r, Scene::dungeon_designer); // clear old map

  const auto& data_e = get_first<SINGLE_DuckgameToDungeon>(r);
  if (data_e == entt::null) {
    std::cout << "not generating dungeon. SINGLE_DuckgameToDungeon not set." << std::endl;
    return;
  }
  const auto& data = r.get<SINGLE_DuckgameToDungeon>(data_e);
  std::cout << "generating dungeon... you hit a patrol! strength: " << data.patrol_that_you_hit.strength << std::endl;

  // todo: make strength impact the number of things spawned
  const int strength = data.patrol_that_you_hit.strength;

  // static int seed = 0;
  // seed++; // Increase seed everytime a map is generated
  engine::RandomState rnd(i);

  DungeonGenerationCriteria dungeon_parameters;
  dungeon_parameters.max_rooms = 30;
  dungeon_parameters.room_size_min = glm::max(dungeon_parameters.room_size_min, 0);
  dungeon_parameters.room_size_max = glm::min(dungeon_parameters.room_size_max, map.xmax);

  const DungeonGenerationResults result = generate_rooms(r, dungeon_parameters, rnd);

  // keep a reference to all the lines as walls, so that we dont have any overlaps.
  std::vector<Line> lines;
  instantiate_walls(r, lines, result, i);
  instantiate_tunnels(r, lines, result);

  // Update pathfinding
  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    if (result.wall_or_floors[idx] == 1) {
      auto& ents = map.map[idx];

      const auto e = create_empty<PathfindComponent>(r);
      r.emplace<PathfindComponent>(e, -1);
      ents.push_back(e);
    }
  }

  // Steps after initial initialization...
  set_generated_entity_positions(r, result, rnd);
  set_player_positions(r, result, rnd);

  // set exit door position
  // bug: exit placed on generated entities. not so bad?
  // Room& room = d.rooms[d.rooms.size() - 1];
  // glm::ivec2 middle = room_center(room);
  // create_dungeon_entity(editor, game, EntityType::tile_type_exit, middle);

  // Set the cost for all of the tiles
  // for (int i = 0; i < dungeon_result.wall_or_floors.size(); i++) {
  //   // const auto grid_xy = engine::grid::index_to_grid_position(i, map.xmax, map.ymax);
  //   const auto empty_e = create_gameplay(r, EntityType::empty_no_transform);
  //   map.map[i].push_back(empty_e);
  //   auto& pathfinding_c = r.emplace<PathfindComponent>(empty_e);
  //   if (dungeon_result.wall_or_floors[i] == 1)
  //     pathfinding_c.cost = -1; // impassable wall
  //   else
  //     pathfinding_c.cost = 0; // floor
  // }

  const auto e = create_empty<DungeonGenerationResults>(r);
  r.emplace<DungeonGenerationResults>(e, result);
  std::cout << "dungeon generated" << std::endl;
}

} // namespace game2d