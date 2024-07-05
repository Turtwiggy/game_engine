#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "helpers/line.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/gen_dungeons//helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/gen_dungeons/helpers/gen_players.hpp"
#include "modules/grid/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_combat_turnbased/components.hpp"

#include "imgui.h"

#include <SDL_scancode.h>
#include <fmt/core.h>
#include <vector>

namespace game2d {

void
update_gen_dungeons_system(entt::registry& r, const glm::ivec2& mouse_pos)
{

  // ImGui::Begin("Debug__Tunnels");
  // for (const auto& [e, t_c] : r.view<Tunnel>().each()) {
  //   ImGui::Separator();
  //   ImGui::Text("TunnelLine0");
  //   for (const auto& t : t_c.line_0)
  //     ImGui::Text("%i %i", t.first, t.second);
  //   ImGui::Text("TunnelLine1");
  //   for (const auto& t : t_c.line_1)
  //     ImGui::Text("%i %i", t.first, t.second);
  // }
  // ImGui::End();

  // static int i = 0;
  // if (get_key_down(input, SDL_SCANCODE_O))
  //   i--;
  // if (get_key_down(input, SDL_SCANCODE_P))
  //   i++;

  bool show_debug_dungeongen = false;
#if defined(_DEBUG)
  show_debug_dungeongen = true;
#endif

  if (show_debug_dungeongen) {
    // ImGui::Text("i: %i", i);

    const auto map_e = get_first<MapComponent>(r);
    if (map_e != entt::null) {
      const auto& map = get_first_component<MapComponent>(r);
      ImGui::Begin("DebugDungeonGen");

      std::vector<Room> rooms;
      for (const auto& [e, room] : r.view<Room>().each())
        rooms.push_back(room);
      ImGui::Text("Rooms: %i", rooms.size());

      std::vector<Tunnel> tunnels;
      for (const auto& [e, t] : r.view<Tunnel>().each())
        tunnels.push_back(t);
      ImGui::Text("Tunnels: %i", tunnels.size());

      const auto grid_pos = engine::grid::world_space_to_grid_space(mouse_pos, map.tilesize);
      ImGui::Text("grid_pos: %i %i", grid_pos.x, grid_pos.y);

      const auto [in_room, room] = inside_room(map, rooms, grid_pos);
      ImGui::Text("Inside Room: %i", in_room);

      const auto in_tunnel = inside_tunnel(tunnels, grid_pos);
      ImGui::Text("Inside Tunnel: %i", in_tunnel);

      const auto dungeon_e = get_first<DungeonGenerationResults>(r);
      if (dungeon_e != entt::null) {
        const auto dungeon_results = get_first_component<DungeonGenerationResults>(r);
        ImGui::Text("Have dungeon gen results");

        auto mouse_idx = engine::grid::grid_position_to_index(grid_pos, map.xmax);
        mouse_idx = glm::clamp(mouse_idx, 0, map.xmax * map.ymax - 1);
        ImGui::Text("mouse: %i is_wall_or_floor: %i", mouse_idx, dungeon_results.wall_or_floors[mouse_idx]);
      }
      ImGui::End();
    }
  }

#if defined(_DEBUG)
  // HACK: force regenerate dungeon
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_I))
    create_empty<RequestGenerateDungeonComponent>(r);
#endif

  const auto& requests = r.view<RequestGenerateDungeonComponent>();
  if (requests.size() == 0)
    return;
  r.destroy(requests.begin(), requests.end()); // process request

  move_to_scene_start(r, Scene::dungeon_designer); // clear old map

  const auto& data_e = get_first<OverworldToDungeonInfo>(r);
  if (data_e == entt::null) {
    fmt::println("not generating dungeon. OverworldToDungeonInfo not set.");
    return;
  }
  const auto& data = r.get<OverworldToDungeonInfo>(data_e);
  fmt::println("generating dungeon... you hit a patrol! strength: {}", data.patrol_that_you_hit.strength);

  // todo: make strength impact the number of things spawned
  const int strength = data.patrol_that_you_hit.strength;

  static int seed = 0;
  seed++; // Increase seed everytime a map is generated
  engine::RandomState rnd(seed);
  const int i = seed;

  // re-generate map
  destroy_first_and_create<MapComponent>(r);
  auto& map = get_first_component<MapComponent>(r);
  // create new map & dungeon constraints
  const int map_width = strength * 100;
  const int map_height = strength * 100;
  map.tilesize = 50;
  map.xmax = map_width / map.tilesize;
  map.ymax = map_height / map.tilesize;
  map.map.resize(map.xmax * map.ymax);

  if (true) {
    const int w = map_width;
    const int h = map_height;
    const int tilesize = map.tilesize;
    const int half_tile_size = tilesize / 10.0f;
    const auto wall_l = create_gameplay(r, EntityType::solid_wall);
    set_position(r, wall_l, { 0, h / 2.0f });
    set_size(r, wall_l, { half_tile_size, h });
    const auto wall_r = create_gameplay(r, EntityType::solid_wall);
    set_position(r, wall_r, { w, h / 2.0f });
    set_size(r, wall_r, { half_tile_size, h });
    const auto wall_u = create_gameplay(r, EntityType::solid_wall);
    set_position(r, wall_u, { w / 2.0f, 0 });
    set_size(r, wall_u, { w, half_tile_size });
    const auto wall_d = create_gameplay(r, EntityType::solid_wall);
    set_position(r, wall_d, { w / 2.0f, h });
    set_size(r, wall_d, { w, half_tile_size });
  }

  DungeonGenerationCriteria dungeon_parameters;
  dungeon_parameters.max_rooms = 30;
  dungeon_parameters.room_size_min = glm::max(strength / 2, 3);
  dungeon_parameters.room_size_max = glm::min((strength / 2) + 5, map.xmax);

  auto result = generate_rooms(r, dungeon_parameters, rnd);

  // keep a reference to all the lines as walls, so that we dont have any overlaps.
  std::vector<Line> lines;
  instantiate_walls(r, lines, result, i);
  instantiate_tunnels(r, lines, result);
  generate_edges(r, map, result);

  // Update pathfinding
  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    if (result.wall_or_floors[idx] == 1) {
      auto& ents = map.map[idx];

      const auto e = create_empty<PathfindComponent>(r, PathfindComponent{ -1 });
      ents.push_back(e);
    }
  }

  // Debug edges
  for (const auto& edge : map.edges) {
    const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
    const auto ga = engine::grid::grid_space_to_world_space(glm::ivec2(edge.cell_a.x, edge.cell_a.y), map.tilesize) + offset;
    const auto gb = engine::grid::grid_space_to_world_space(glm::ivec2(edge.cell_b.x, edge.cell_b.y), map.tilesize) + offset;
    const auto l = generate_line({ ga.x, ga.y }, { gb.x, gb.y }, 1);
    const entt::entity e = create_gameplay(r, EntityType::empty_with_transform);
    set_transform_with_line(r, e, l);
    r.get<TagComponent>(e).tag = "debugline";
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

  create_empty<DungeonGenerationResults>(r, result);
  fmt::println("dungeon generated");
}

} // namespace game2d