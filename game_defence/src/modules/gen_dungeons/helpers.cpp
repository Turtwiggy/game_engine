#include "helpers.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "helpers/line.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ui_spaceship_designer/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/transform.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace game2d {

// update walls_or_floors with room
void
create_room(const Room& room, std::vector<int>& wall_or_floors, const int xmax)
{
  const auto& w = room.aabb.size.x;
  const auto& h = room.aabb.size.y;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {

      // create walls on the outside of the room
      bool create_as_wall = false;
      if (x == 0 || x == w - 1)
        create_as_wall = true;
      if (y == 0 || y == h - 1)
        create_as_wall = true;

      const auto grid_pos = glm::ivec2{ room.tl.x + x, room.tl.y + y };
      const int global_idx = engine::grid::grid_position_to_index(grid_pos, xmax);
      wall_or_floors[global_idx] = create_as_wall ? 1 : 0; // else floor
    }
  }
};

// returns two lines: one horizontal, one vetical
Tunnel
create_tunnel(entt::registry& r, const Room& room, const Room& prev_room, const int xmax, engine::RandomState& rnd)
{
  const int zero_or_one = int(engine::rand_det_s(rnd.rng, 0, 1));
  const bool horizontal_then_vertical = zero_or_one == 1;

  const auto x1 = room.aabb.center.x;
  const auto y1 = room.aabb.center.y;
  const auto x2 = prev_room.aabb.center.x;
  const auto y2 = prev_room.aabb.center.y;

  int corner_x = 0;
  int corner_y = 0;
  if (horizontal_then_vertical) {
    corner_x = x2;
    corner_y = y1;
  } else
  // move vertically, then horizontally
  {
    corner_x = x1;
    corner_y = y2;
  }

  // generate lines via bresenham line algorithm
  const auto line_0 = create_line(room.aabb.center.x, room.aabb.center.y, corner_x, corner_y);
  const auto line_1 = create_line(corner_x, corner_y, prev_room.aabb.center.x, prev_room.aabb.center.y);

  Tunnel t;
  t.horizontal_then_vertical = horizontal_then_vertical;
  t.line_0 = line_0;
  t.line_1 = line_1;
  t.room = room;
  t.prev_room = prev_room;
  return t;
};

// room generation algorithm
//
DungeonGenerationResults
generate_rooms(entt::registry& r, const DungeonGenerationCriteria& data, engine::RandomState& rnd)
{
  const auto& map = get_first_component<MapComponent>(r);

  std::vector<int> wall_or_floors(map.xmax * map.ymax, 1); // 1: everything as wall

  const int xmax = map.xmax;
  const int max_rooms = data.max_rooms;
  const int room_size_min = data.room_size_min;
  const int room_size_max = data.room_size_max;

  // Create some rooms
  const int map_width = map.xmax;
  const int map_height = map.ymax;

  std::vector<Room> rooms;
  std::vector<Tunnel> tunnels;

  for (int i = 0; i < max_rooms; i++) {
    //
    // the -1 is so that when we offset to center of the tiles, visually the room doesnt go outside the grid
    const int rnd_width_x = static_cast<int>(engine::rand_det_s(rnd.rng, room_size_min, room_size_max + 1)) - 1;
    const int rnd_width_y = static_cast<int>(engine::rand_det_s(rnd.rng, room_size_min, room_size_max + 1)) - 1;
    const int gen_room_width = static_cast<int>(glm::roundEven(static_cast<float>(rnd_width_x)));
    const int gen_room_height = static_cast<int>(glm::roundEven(static_cast<float>(rnd_width_y)));

    const int rnd_tl_x = engine::rand_det_s(rnd.rng, 0, map_width - gen_room_width - 1);
    const int rnd_tl_y = engine::rand_det_s(rnd.rng, 0, map_height - gen_room_height - 1);
    const auto tl = glm::ivec2{ rnd_tl_x, rnd_tl_y };

    Room room;
    room.tl = tl;
    room.aabb.center = { tl.x + (gen_room_width / 2), tl.y + (gen_room_height / 2) };
    room.aabb.size = { gen_room_width, gen_room_height };

    // if new room collides with existin groom, skip this room.
    const auto room_collides = [&room](const Room& other) { return collide(room.aabb, other.aabb); };
    const auto it = std::find_if(rooms.begin(), rooms.end(), room_collides);
    if (it != rooms.end())
      continue;
    const auto& w = gen_room_width;
    const auto& h = gen_room_height;
    std::cout << "gen room. size: " << w << ", " << h << " at " << room.tl.x << ", " << room.tl.y << std::endl;

    // room stored in the wall_of_floors bitmap
    create_room(room, wall_or_floors, xmax);
    rooms.push_back(room);

    // dig out some TUNNELS between this room and the previous one
    if (i == 0) // starting room cant build a tunnel :()
      continue;

    const Room& prev_room = rooms[rooms.size() - 2];
    const auto tunnel = create_tunnel(r, room, prev_room, xmax, rnd);

    // a) x1, y1 to corner_x, corner_y
    for (const auto& [x, y] : tunnel.line_0)
      wall_or_floors[engine::grid::grid_position_to_index({ x, y }, xmax)] = 0; // set as floor

    // b) corner_x, corner_y to x2, y2
    for (const auto& [x, y] : tunnel.line_1)
      wall_or_floors[engine::grid::grid_position_to_index({ x, y }, xmax)] = 0; // set as floor

    tunnels.push_back(tunnel);

    // if (rooms.size() == 2)
    //   break; // DEBUG: only X rooms

  } // end iterating rooms

  DungeonGenerationResults results;
  results.rooms = rooms;
  results.tunnels = tunnels;
  results.wall_or_floors = wall_or_floors;
  return results;
};

entt::entity
create_wall(entt::registry& r, const glm::ivec2& pos, const glm::ivec2& size, const entt::entity& parent)
{
  const auto wall_e = create_gameplay(r, EntityType::solid_wall);
  set_position(r, wall_e, pos);
  set_size(r, wall_e, size);
  r.get<TagComponent>(wall_e).tag = "wall";

  Wall w;
  w.parent_room = parent;
  r.emplace<Wall>(wall_e, w);

  return wall_e;
};

void
instantiate_tunnels(entt::registry& r, const DungeonGenerationResults& results)
{
  const auto& map = get_first_component<MapComponent>(r);

  for (const auto& tunnel : results.tunnels) {

    const auto create_square = [&r, &map](const std::pair<int, int>& a, const std::pair<int, int>& b) {
      const auto tunnel_to_create = create_gameplay(r, EntityType::empty_with_transform);

      const engine::SRGBColour col = { 1.0f, 0.0f, 0.0f, 0.1f };
      set_colour(r, tunnel_to_create, col);
      r.emplace<DefaultColour>(tunnel_to_create, col);

      const int grid_w = glm::abs(b.first - a.first) + 1;
      const int grid_h = glm::abs(b.second - a.second) + 1;
      const glm::ivec2 gridspace_tl = { glm::min(a.first, b.first), glm::min(a.second, b.second) };

      const auto& worldspace_tl = gridspace_tl * map.tilesize;
      const auto worldspace_size = glm::ivec2{ grid_w, grid_h } * map.tilesize;
      const int w = worldspace_size.x;
      const int h = worldspace_size.y;

      // const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
      const glm::ivec2 offset = { 0, 0 };

      const glm::ivec2 tl = glm::ivec2{ worldspace_tl } + offset;
      const glm::ivec2 tr = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y) } + offset;
      const glm::ivec2 bl = glm::ivec2{ (worldspace_tl.x), (worldspace_tl.y + h) } + offset;
      const glm::ivec2 br = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y + h) } + offset;
      const glm::ivec2 worldspace_center = { (tl.x + tr.x) / 2.0f, (tr.y + br.y) / 2.0f };

      const int lw = 4;
      const auto w0 = create_wall(r, { worldspace_center.x, tl.y }, { w, lw }, tunnel_to_create); // tl, tr
      const auto w1 = create_wall(r, { tr.x, worldspace_center.y }, { lw, h }, tunnel_to_create); // tr, br
      const auto w2 = create_wall(r, { worldspace_center.x, br.y }, { w, lw }, tunnel_to_create); // br, bl
      const auto w3 = create_wall(r, { bl.x, worldspace_center.y }, { lw, h }, tunnel_to_create); // bl, tl

      set_position(r, tunnel_to_create, worldspace_center);
      set_size(r, tunnel_to_create, worldspace_size);

      return tunnel_to_create;
    };

    const auto& square_0 = tunnel.line_0;
    const auto& square_1 = tunnel.line_1;

    if (square_0.size() >= 1) {               // first can be last
      const auto square_pFirst = square_0[0]; // could be horizontal or vertical
      const auto square_pLast = square_0[square_0.size() - 1];
      create_square(square_pFirst, square_pLast);
    }

    if (square_1.size() >= 1) {               // first can be last
      const auto square_pFirst = square_1[0]; // could be horizontal or vertical
      const auto square_pLast = square_1[square_1.size() - 1];
      create_square(square_pFirst, square_pLast);
    }

    r.emplace<Tunnel>(r.create(), tunnel);
  }
};

void
instantiate_walls(entt::registry& r, const DungeonGenerationResults& results)
{
  const auto& map = get_first_component<MapComponent>(r);

  // for (int i = 0; i < results.wall_or_floors.size(); i++) {
  //   if (results.wall_or_floors[i] == 1) {

  //     // optimization: dont create a square if surrounded by walls
  //     // std::vector<std::pair<engine::grid::GridDirection, int>> neighbours;
  //     // const glm::ivec2 grid_xy = engine::grid::index_to_grid_position(i, map.xmax, map.ymax);
  //     // engine::grid::get_neighbour_indicies_with_diagonals(grid_xy.x, grid_xy.y, map.xmax, map.ymax, neighbours);
  //     // bool surrounded_by_walls = true;
  //     // for (const auto [dir, idx] : neighbours)
  //     //   surrounded_by_walls &= results.wall_or_floors[idx] == 1;
  //     // if (surrounded_by_walls)
  //     //   continue; // dont render

  //     const glm::ivec2 pos = engine::grid::index_to_world_position(i, map.xmax, map.ymax, map.tilesize);
  //     const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };

  //     const auto e = create_gameplay(r, EntityType::solid_wall);
  //     set_position(r, e, pos + offset);
  //     set_size(r, e, glm::ivec2{ map.tilesize, map.tilesize });
  //   }
  // }

  for (const Room& room : results.rooms) {
    const auto room_to_create = create_gameplay(r, EntityType::empty_with_transform);
    // const auto e = create_gameplay(r, EntityType::empty_no_transform);

    // floor
    const engine::SRGBColour col = { 1.0f, 1.0f, 1.0f, 0.02f };
    set_colour(r, room_to_create, col);
    r.emplace<DefaultColour>(room_to_create, col);

    // create room walls
    const auto& gridspace_tl = room.tl;
    const auto& worldspace_tl = gridspace_tl * map.tilesize;
    const auto worldspace_size = room.aabb.size * map.tilesize;

    // const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
    const glm::ivec2 offset = { 0, 0 };

    const int w = worldspace_size.x;
    const int h = worldspace_size.y;
    const glm::ivec2 tl = glm::ivec2{ worldspace_tl } + offset;
    const glm::ivec2 tr = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y) } + offset;
    const glm::ivec2 bl = glm::ivec2{ (worldspace_tl.x), (worldspace_tl.y + h) } + offset;
    const glm::ivec2 br = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y + h) } + offset;
    const glm::ivec2 worldspace_center = { (tl.x + tr.x) / 2.0f, (tr.y + br.y) / 2.0f };

    const int lw = 4;
    const auto w0 = create_wall(r, { worldspace_center.x, tl.y }, { w, lw }, room_to_create); // tl, tr
    const auto w1 = create_wall(r, { tr.x, worldspace_center.y }, { lw, h }, room_to_create); // tr, br
    const auto w2 = create_wall(r, { worldspace_center.x, br.y }, { w, lw }, room_to_create); // br, bl
    const auto w3 = create_wall(r, { bl.x, worldspace_center.y }, { lw, h }, room_to_create); // bl, tl

    r.replace<Wall>(w0, Wall{ tl, tr, room_to_create });
    r.replace<Wall>(w1, Wall{ tr, br, room_to_create });
    r.replace<Wall>(w2, Wall{ br, bl, room_to_create });
    r.replace<Wall>(w3, Wall{ bl, tl, room_to_create });

    set_position(r, room_to_create, worldspace_center);
    set_size(r, room_to_create, worldspace_size);
    r.emplace<Room>(room_to_create, room);
  }
};

void
set_generated_entity_positions(entt::registry& r, const DungeonGenerationResults& results, engine::RandomState& rnd)
{
  const auto& map_c = get_first_component<MapComponent>(r);
  const auto& data = get_first_component<SINGLE_DuckgameToDungeon>(r);
  const int strength = data.patrol_that_you_hit.strength;

  const auto& rooms_view = r.view<Room>();
  const int amount_of_rooms = rooms_view.size();
  // const int potential_items_per_room = 5 + (floor);
  // const int potential_monsters_per_room = 5 + (floor);
  // printf("floor %i has max items %i, max monsters: %i\n", floor, potential_items_per_room, potential_monsters_per_room);

  for (int room_idx = 0; const auto& [e, room] : rooms_view.each()) {
    // first room is player room
    if (room_idx == 0) {
      room_idx++;
      continue;
    }
    room_idx++;

    // stop spawning entities.
    // you've spawned enough for your strength.
    if ((room_idx - 1) > strength)
      continue;

    const glm::ivec2 tl = room.tl;
    const glm::ivec2 br = room.tl + glm::ivec2{ room.aabb.size.x, room.aabb.size.y };

    // ramdomly place generated entities
    for (int i = 0; i < 1; i++) {
      const int x = static_cast<int>(engine::rand_det_s(rnd.rng, tl.x + 1, br.x - 1));
      const int y = static_cast<int>(engine::rand_det_s(rnd.rng, tl.y + 1, br.y - 1));
      const glm::ivec2 grid_index = { x, y };

      // // Check the tile isn't occupied
      // const auto full =
      //   std::find_if(d.occupied.begin(), d.occupied.end(), [&grid_index](const std::pair<entt::entity, glm::ivec2>&
      //   other)
      //   {
      //     return other.second == grid_index;
      //   });
      // if (full != d.occupied.end())
      //   continue; // entity already at position

      const glm::ivec2 worldspace = engine::grid::grid_space_to_world_space(grid_index, map_c.tilesize);
      const glm::ivec2 offset = { map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
      const glm::ivec2 pos = worldspace + offset;

      CombatEntityDescription desc;
      desc.position = pos;
      desc.team = AvailableTeams::enemy;
      const auto e = create_combat_entity(r, desc);

      // when to increase the generated monsters items stats?
    }
  }
};

// TODO: replace with the number of player's crew
// TODO: check if occupied
void
set_player_positions(entt::registry& r, const DungeonGenerationResults& results, engine::RandomState& rnd)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  for (const auto& [e, room] : r.view<Room>().each()) {
    const glm::ivec2 tl = room.tl;
    const glm::ivec2 br = room.tl + glm::ivec2{ room.aabb.size.x, room.aabb.size.y };
    const glm::ivec2 worldspace_center = engine::grid::grid_space_to_world_space(room.aabb.center, map_c.tilesize);

    // set camera on this room center
    const auto& camera_e = get_first<OrthographicCamera>(r);
    auto& camera_t = r.get<TransformComponent>(camera_e);
    camera_t.position = { worldspace_center.x, worldspace_center.y, 0.0 };

    // place players in first room

    for (int i = 0; i < 1; i++) {
      const int x = static_cast<int>(engine::rand_det_s(rnd.rng, tl.x + 1, br.x - 1));
      const int y = static_cast<int>(engine::rand_det_s(rnd.rng, tl.y + 1, br.y - 1));
      const glm::ivec2 grid_index = { x, y };
      const glm::ivec2 worldspace = engine::grid::grid_space_to_world_space(grid_index, map_c.tilesize);
      const glm::ivec2 offset = { map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
      const glm::ivec2 pos = worldspace + offset;

      CombatEntityDescription desc;
      desc.position = pos;
      desc.team = AvailableTeams::player;
      const auto e = create_combat_entity(r, desc);
    }

    break; // only spawn players in first room
  }
};

} // namespace game2d
