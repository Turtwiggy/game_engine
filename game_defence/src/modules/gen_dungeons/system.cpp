#include "system.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/gen_dungeons//helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/gen_dungeons/helpers/gen_players.hpp"
#include "modules/grid/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/system_change_gun_z_index/helpers.hpp"
#include "modules/system_fov/components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "sprites/helpers.hpp"

#include "imgui.h"

#include <SDL_scancode.h>
#include <fmt/core.h>
#include <vector>

namespace game2d {

void
update_gen_dungeons_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  bool show_debug_dungeongen = true;
#if defined(_DEBUG)
  show_debug_dungeongen = true;
#endif

  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  static int i = 0;

  if (show_debug_dungeongen) {
    // ImGui::Text("i: %i", i);
    ImGui::Begin("DebugDungeonGen", &show_debug_dungeongen);

    const auto map_e = get_first<MapComponent>(r);
    if (map_e != entt::null) {
      const auto& map = get_first_component<MapComponent>(r);

      std::vector<Room> rooms;
      for (const auto& [e, room] : r.view<Room>().each())
        rooms.push_back(room);
      ImGui::Text("Rooms: %zu", rooms.size());

      std::vector<Tunnel> tunnels;
      for (const auto& [e, t] : r.view<Tunnel>().each())
        tunnels.push_back(t);
      ImGui::Text("Tunnels: %zu", tunnels.size());

      // for (const auto& [e, t_c] : r.view<Tunnel>().each()) {
      //   ImGui::Separator();
      //   ImGui::Text("TunnelLine0");
      //   for (const auto& t : t_c.line_0)
      //     ImGui::Text("%i %i", t.first, t.second);
      //   ImGui::Text("TunnelLine1");
      //   for (const auto& t : t_c.line_1)
      //     ImGui::Text("%i %i", t.first, t.second);
      // }

      const auto grid_pos = engine::grid::worldspace_to_grid_space(mouse_pos, map.tilesize);
      ImGui::Text("grid_pos: %i %i", grid_pos.x, grid_pos.y);
      ImGui::Text("grid_idx: %i", engine::grid::grid_position_to_index(grid_pos, map.xmax));

      const auto [in_room, room] = inside_room(map, rooms, grid_pos);
      ImGui::Text("Inside Room: %i", in_room);

      const auto ts = inside_tunnels(tunnels, grid_pos);
      ImGui::Text("Tunnels Size: %i", int(ts.size()));

      if (get_key_down(input, SDL_SCANCODE_KP_1))
        i--;
      if (get_key_down(input, SDL_SCANCODE_KP_2))
        i++;
      ImGui::Text("Increase/decerase, kp 1/2");
      ImGui::Text("Generate kp0");
      ImGui::Text("Seed: %i", i);

      // HACK: force regenerate dungeon
      if (get_key_down(input, SDL_SCANCODE_KP_0))
        create_empty<RequestGenerateDungeonComponent>(r);

      if (get_mouse_mmb_press()) {
        const auto worldpos = engine::grid::grid_space_to_world_space(grid_pos, map.tilesize);
        const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
        set_position(r, get_first<PlayerComponent>(r), worldpos + offset);
      }

      ImGui::SeparatorText("Map");
      const auto mouse_idx = engine::grid::grid_position_to_clamped_index(grid_pos, map.xmax, map.ymax);
      ImGui::Text("Clamped idx: %i", mouse_idx);
      const auto idx_is_null = map.map[mouse_idx] == entt::null;
      ImGui::Text("Map ent is null: %i", idx_is_null);
      ImGui::Text("Map ent is visible: %i", !idx_is_null && r.try_get<VisibleComponent>(map.map[mouse_idx]) != nullptr);
      ImGui::Text("Map ent is hovered: %i", !idx_is_null && r.try_get<HoveredComponent>(map.map[mouse_idx]) != nullptr);

      const auto dungeon_e = get_first<DungeonGenerationResults>(r);
      if (dungeon_e != entt::null) {
        const auto dungeon = get_first_component<DungeonGenerationResults>(r);
        ImGui::Text("Have dungeon gen results");

        const auto floor_idx_is_null = dungeon.floor_tiles[mouse_idx] == entt::null;
        ImGui::Text("Floor ent is null: %i", floor_idx_is_null);
        ImGui::Text("Floor ent is visible: %i",
                    !floor_idx_is_null && r.try_get<VisibleComponent>(dungeon.floor_tiles[mouse_idx]) != nullptr);
        ImGui::Text("Floor ent is hovered: %i",
                    !floor_idx_is_null && r.try_get<HoveredComponent>(dungeon.floor_tiles[mouse_idx]) != nullptr);
      }
    }

    ImGui::End();
  }

  const auto& requests = r.view<RequestGenerateDungeonComponent>();
  if (requests.size() == 0)
    return;
  r.destroy(requests.begin(), requests.end()); // process request

  move_to_scene_start(r, Scene::dungeon_designer); // clear old map

  // cleanup

  const auto& data_e = get_first<OverworldToDungeonInfo>(r);
  if (data_e == entt::null) {
    fmt::println("not generating dungeon. OverworldToDungeonInfo not set.");
    return;
  }
  // const auto& data = r.get<OverworldToDungeonInfo>(data_e);
  fmt::println("generating dungeon... todo: generate of a certain strength");

  // static int seed = 0;
  // seed++; // Increase seed everytime a map is generated
  int seed = i;
  engine::RandomState rnd(seed);

  // re-generate map
  destroy_first_and_create<MapComponent>(r);
  auto& map = get_first_component<MapComponent>(r);

  // create new map & dungeon constraints
  const int strength = 10; // TODO: fix this
  const int map_width = strength * 100;
  const int map_height = strength * 100;
  map.tilesize = 50;
  map.xmax = map_width / map.tilesize;
  map.ymax = map_height / map.tilesize;
  map.map.resize(map.xmax * map.ymax, entt::null);

  DungeonGenerationCriteria dungeon_parameters;
  dungeon_parameters.max_rooms = 30;
  dungeon_parameters.room_size_min = glm::max(strength / 2, 3);
  dungeon_parameters.room_size_max = glm::min((strength / 2) + 5, map.xmax);

  auto result = generate_rooms(r, dungeon_parameters, rnd);

  // give to entt?
  for (const Room& room : result.rooms)
    create_empty<Room>(r, room);
  for (const Tunnel& tunnel : result.tunnels)
    create_empty<Tunnel>(r, tunnel);

  // generate edges for pathfinding.
  // they get put in the map.edges
  generate_edges(r, map, result);

  // Update pathfinding with walls
  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    if (result.wall_or_floors[idx] == 1) {
      const auto e = create_empty<PathfindComponent>(r, { PathfindComponent{ -1 } });

      if (map.map[idx] != entt::null) {
        fmt::println("ERROR: wall index isnt empty");
        exit(1); // crash
      }

      map.map[idx] = e;
    }
  }

  // add floor tiles
  result.floor_tiles.resize(result.wall_or_floors.size(), entt::null);
  for (size_t xy = 0; xy < result.wall_or_floors.size(); xy++) {
    if (result.wall_or_floors[xy] == 0) {
      const auto gridpos = engine::grid::index_to_grid_position(xy, map.xmax, map.ymax);
      const auto floor_e = create_transform(r);
      const glm::ivec2 worldspace = engine::grid::grid_space_to_world_space(gridpos, map.tilesize);
      const glm::ivec2 offset = { map.tilesize / 2.0f, map.tilesize / 2.0f };
      const glm::ivec2 pos = worldspace + offset;
      set_position(r, floor_e, pos);
      set_size(r, floor_e, { map.tilesize, map.tilesize });
      // float rnd_col = engine::rand_det_s(rnd.rng, 0.8, 0.85f);
      // set_colour(r, floor_e, { rnd_col, rnd_col, rnd_col, 1.0f });
      set_z_index(r, floor_e, -1);
      result.floor_tiles[engine::grid::grid_position_to_index(gridpos, map.xmax)] = floor_e;
    }
  }

  instantiate_edges(r, map);

  // Steps after initial initialization...
  set_generated_entity_positions(r, result, rnd);

  set_player_positions(r, result, rnd);
  // DataJetpackActor desc;
  // desc.pos = { -100, -100 };
  // desc.team = AvailableTeams::player;
  // const auto e = Factory_DataJetpackActor::create(r, desc);
  // r.emplace<CameraFollow>(e);

  // give helmet to breathe
  const auto helmet_e = create_transform(r);
  set_sprite(r, helmet_e, "HELMET_5");
  set_size(r, helmet_e, { 32, 32 });
  set_z_index(r, helmet_e, 2); // above player
  r.emplace<DynamicTargetComponent>(helmet_e).target = get_first<PlayerComponent>(r);
  SetPositionAtDynamicTarget tgt;
  tgt.offset = { 0, -10 };
  r.emplace<SetPositionAtDynamicTarget>(helmet_e, tgt);
  set_colour(r, helmet_e, { 1.0f, 1.0f, 1.0f, 1.0f });

  // set exit door position
  // bug: exit placed on generated entities. not so bad?
  // Room& room = d.rooms[d.rooms.size() - 1];
  // glm::ivec2 middle = room_center(room);
  // create_dungeon_entity(editor, game, EntityType::tile_type_exit, middle);

  // Set the cost for all of the tiles
  // for (size_t i = 0; i < dungeon_result.wall_or_floors.size(); i++) {
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