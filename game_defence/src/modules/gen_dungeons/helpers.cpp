#include "helpers.hpp"

#include "actors.hpp"
#include "colour/colour.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "helpers/line.hpp"
#include "imgui.h"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/components.hpp"

#include <algorithm>
#include <fmt/core.h>
#include <vector>

namespace game2d {

// returns two lines: one horizontal, one vetical
Tunnel
generate_tunnel(entt::registry& r, const Room& room, const Room& prev_room, const int xmax, engine::RandomState& rnd)
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
    fmt::println("gen room. size: {}, {} at {}, {}", w, h, room.tl.x, room.tl.y);

    // update_wall_of_floors_bitmap
    auto create_room = [](const Room& room, std::vector<int>& wall_or_floors, const int xmax) {
      const auto& w = room.aabb.size.x;
      const auto& h = room.aabb.size.y;

      for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {

          // by commenting out below,
          // set the entire generated room to "floor"

          // create walls on the outside of the room
          bool create_as_wall = false;
          if (x == 0 || x == w - 1)
            create_as_wall = true;
          if (y == 0 || y == h - 1)
            create_as_wall = true;

          const auto grid_pos = glm::ivec2{ room.tl.x + x, room.tl.y + y };
          const auto global_idx = engine::grid::grid_position_to_index(grid_pos, xmax);
          wall_or_floors[global_idx] = 0;
        }
      }
    };
    create_room(room, wall_or_floors, xmax);
    rooms.push_back(room);

    // dig out some TUNNELS between this room and the previous one
    if (i == 0) // starting room cant build a tunnel :()
      continue;

    const Room& prev_room = rooms[rooms.size() - 2];
    const auto tunnel = generate_tunnel(r, room, prev_room, xmax, rnd);

    // a) x1, y1 to corner_x, corner_y
    for (const auto& [x, y] : tunnel.line_0)
      wall_or_floors[engine::grid::grid_position_to_index({ x, y }, xmax)] = 0; // set as floor

    // b) corner_x, corner_y to x2, y2
    for (const auto& [x, y] : tunnel.line_1)
      wall_or_floors[engine::grid::grid_position_to_index({ x, y }, xmax)] = 0; // set as floor

    tunnels.push_back(tunnel);

  } // end iterating rooms

  DungeonGenerationResults results;
  results.rooms = rooms;
  results.tunnels = tunnels;
  results.wall_or_floors = wall_or_floors;
  return results;
};

entt::entity
create_wall(entt::registry& r, const glm::ivec2& pos, const glm::ivec2& size)
{
  const auto wall_e = create_gameplay(r, EntityType::solid_wall);
  set_position(r, wall_e, pos);
  set_size(r, wall_e, size);
  r.get<TagComponent>(wall_e).tag = "dungeon-wall";
  return wall_e;
};

Line
convert_tunnel_line(entt::registry& r, const MapComponent& map, const std::pair<int, int>& a, const std::pair<int, int>& b)
{
  const int grid_w = glm::abs(b.first - a.first) + 1;
  const int grid_h = glm::abs(b.second - a.second) + 1;
  const glm::ivec2 gridspace_tl = { glm::min(a.first, b.first), glm::min(a.second, b.second) };

  const auto& worldspace_tl = gridspace_tl * map.tilesize;
  const auto worldspace_size = glm::ivec2{ grid_w, grid_h } * map.tilesize;
  const int w = worldspace_size.x;
  const int h = worldspace_size.y;

  const glm::ivec2 tl = glm::ivec2{ worldspace_tl };
  const glm::ivec2 tr = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y) };
  const glm::ivec2 bl = glm::ivec2{ (worldspace_tl.x), (worldspace_tl.y + h) };
  const glm::ivec2 br = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y + h) };
  const glm::ivec2 worldspace_center = { (tl.x + tr.x) / 2.0f, (tr.y + br.y) / 2.0f };

  Line l;

  const bool is_horizontal = grid_w > grid_h;
  if (is_horizontal) {
    l.p0 = { tl.x, worldspace_center.y };
    l.p1 = { tr.x, worldspace_center.y };
  } else {
    l.p0 = { worldspace_center.x, tr.y };
    l.p1 = { worldspace_center.x, br.y };
  }

  make_p1_bigger_point(l);
  return l;
};

// one tunnel is two lines
std::vector<Line>
convert_tunnel_to_lines(entt::registry& r, const MapComponent& map, const Tunnel& t)
{
  std::vector<Line> results;

  // Create a line along the tunnel.
  const auto& square_0 = t.line_0;
  const auto& square_1 = t.line_1;
  // square_0 or square_1 could be horizontal or vertical

  if (square_0.size() >= 1) {
    const auto square_pFirst = square_0[0];
    const auto square_pLast = square_0[square_0.size() - 1];
    auto l = convert_tunnel_line(r, map, square_pFirst, square_pLast);
    results.push_back(l);
  }

  if (square_1.size() >= 1) {
    const auto square_pFirst = square_1[0];
    const auto square_pLast = square_1[square_1.size() - 1];
    auto l = convert_tunnel_line(r, map, square_pFirst, square_pLast);
    results.push_back(l);
  }

  return results;
};

Line
shrink_line_at_each_end(const Line& non_shrunk, const int shrinkage)
{
  Line l = non_shrunk;
  const bool horizontal = is_horizontal(l);
  if (horizontal) {
    l.p0.x += shrinkage;
    l.p1.x -= shrinkage;
  } else {
    l.p0.y += shrinkage;
    l.p1.y -= shrinkage;
  }
  return l;
};

void
instantiate_walls(entt::registry& r, std::vector<Line>& lines, DungeonGenerationResults& results, const int& i)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto& tunnels = results.tunnels;
  const float half_tilesize = map.tilesize / 2.0f;

  std::vector<Line> lines_to_instantiate;

  int room_count = 0;
  for (const Room& room : results.rooms) {
    // if (i == room_count)
    //   break; // debug room gen algorithm

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

    // Convert room to 4 lines
    const Line l0{ tl, tr }; // horizontal
    const Line l1{ tr, br }; // vertical
    const Line l2{ bl, br }; // horizontal
    const Line l3{ tl, bl }; // vertical

    const int num_lines_prior = lines_to_instantiate.size();
    lines_to_instantiate.push_back(l0);
    lines_to_instantiate.push_back(l1);
    lines_to_instantiate.push_back(l2);
    lines_to_instantiate.push_back(l3);

    // A tunnel consists of 2 lines.
    for (const Tunnel& t : tunnels) {
      const auto two_tunnel_lines = convert_tunnel_to_lines(r, map, t);
      for (const auto& t_line : two_tunnel_lines) {

        // shrink line so it doesnt collide at ends
        const Line t_line_shrunk = shrink_line_at_each_end(t_line, 5);

        const auto collisions = get_line_collisions(lines_to_instantiate, t_line_shrunk);

        // Match tunnel collisions to lines_to_instantiate walls.
        for (const auto& [coll_line, coll_intersection] : collisions) {

          // remove the line
          {
            const auto cond = [&coll_line](const Line& o) { return o.p0 == coll_line.p0 && o.p1 == coll_line.p1; };
            auto it = std::find_if(lines_to_instantiate.begin(), lines_to_instantiate.end(), cond);
            if (it != lines_to_instantiate.end())
              lines_to_instantiate.erase(it);
          }

          Line line_to_split = coll_line;
          make_p1_bigger_point(line_to_split);

          // split the line in to 2 lines...
          Line new_line_a;
          new_line_a.p0 = line_to_split.p0;
          new_line_a.p1 = coll_intersection;

          Line new_line_b;
          new_line_b.p0 = coll_intersection;
          new_line_b.p1 = line_to_split.p1;

          if (new_line_a.p0 == new_line_a.p1)
            fmt::println("(warning: map gen). Weird collision. Debug this.");
          if (new_line_b.p0 == new_line_b.p1)
            fmt::println("(warning: map gen). Weird collision. Debug this.");
          if (new_line_a.p0.x != new_line_a.p1.x && new_line_a.p0.y != new_line_a.p1.y)
            fmt::println("(warning: map gen). Diagonal line. Debug this.");
          if (new_line_b.p0.x != new_line_b.p1.x && new_line_b.p0.y != new_line_b.p1.y)
            fmt::println("(warning: map gen). Diagonal line. Debug this.");

          // Adjust the new two new lines so that they're half a grid apart
          if (is_horizontal(line_to_split)) {
            new_line_a.p1.x -= half_tilesize;
            new_line_b.p0.x += half_tilesize;
          } else {
            new_line_a.p1.y -= half_tilesize;
            new_line_b.p0.y += half_tilesize;
          }

          const auto line_is_valid = [](const Line& l) -> bool {
            // horizontal means y coordinates are the same
            if (is_horizontal(l))
              return l.p0.x < l.p1.x;
            else
              return l.p0.y < l.p1.y;
          };

          // After adjustments, check if the line went to 0 or negative length
          if (line_is_valid(new_line_a))
            lines_to_instantiate.push_back((new_line_a));
          if (line_is_valid(new_line_b))
            lines_to_instantiate.push_back((new_line_b));

          //
        }
      }
    }

    const int num_lines_post = lines_to_instantiate.size();
    if (num_lines_post - num_lines_prior < 4) // 4 because a 2d room has 4 walls
      fmt::println("error: did not add enough lines for room to be covered.");

    // debug the room
    const auto room_to_create = create_gameplay(r, EntityType::empty_with_transform);
    set_position(r, room_to_create, worldspace_center);
    set_size(r, room_to_create, worldspace_size);
    r.emplace<Room>(room_to_create, room);

    // add some interesting colours
    const auto imcol = (ImVec4)ImColor::HSV(room_count / 7.0f, 0.6f, 0.6f);
    const engine::SRGBColour col = { imcol.x, imcol.y, imcol.z, 0.1f };
    set_colour(r, room_to_create, col);
    r.emplace<DefaultColour>(room_to_create, col);

    // add worldspace text for room to debug it
    // TAG: ROOM_ID UI
    // r.emplace<WorldspaceTextComponent>(room_to_create, std::to_string(room_count));

    room_count++;
  }

  // Instantiate lines
  // const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
  for (const auto& l : lines_to_instantiate) {
    const auto e = create_wall(r, get_center_from_line(l), get_size_from_line(l));
    set_colour(r, e, { 0.0f, 1.0f, 0.0f, 1.0f });
  }

  results.lines_to_instantiate = lines_to_instantiate;
};

void
set_generated_entity_positions(entt::registry& r, DungeonGenerationResults& results, engine::RandomState& rnd)
{
  const auto& map_c = get_first_component<MapComponent>(r);
  const auto& data = get_first_component<OverworldToDungeonInfo>(r);
  const int strength = data.patrol_that_you_hit.strength;
  auto& rooms = results.rooms;

  const int max_attempts_to_generate_unique_spot = 3;
  // const int potential_items_per_room = 5 + (floor);
  // const int potential_monsters_per_room = 5 + (floor);
  // printf("floor %i has max items %i, max monsters: %i\n", floor, potential_items_per_room, potential_monsters_per_room);

  int room_idx_to_spawn = 1;
  for (int i = strength; i > 0; i--) {
    if (room_idx_to_spawn == 0)
      room_idx_to_spawn++; // first index room is player room

    Room& room = rooms[room_idx_to_spawn];
    const glm::ivec2 tl = room.tl;
    const glm::ivec2 br = room.tl + glm::ivec2{ room.aabb.size.x, room.aabb.size.y };

    glm::ivec2 gridpos{ tl.x + 1, tl.y + 1 };
    for (int j = 0; j < max_attempts_to_generate_unique_spot; j++) {
      // random position
      const int x = static_cast<int>(engine::rand_det_s(rnd.rng, tl.x, br.x));
      const int y = static_cast<int>(engine::rand_det_s(rnd.rng, tl.y, br.y));
      const auto attempt = glm::ivec2{ x, y };

      // Check the tile isn't occupied
      const auto full =
        std::find_if(room.occupied.begin(), room.occupied.end(), [&attempt](const auto& pos) { return pos == attempt; });
      const bool entity_at_position = full != room.occupied.end();

      if (entity_at_position)
        continue; // try again

      // otherwise, hit our gen spot
      gridpos = attempt;
      room.occupied.push_back(gridpos);
      break;
    }

    const glm::ivec2 worldspace = engine::grid::grid_space_to_world_space(gridpos, map_c.tilesize);
    const glm::ivec2 offset = { map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
    const glm::ivec2 pos = worldspace + offset;

    CombatEntityDescription desc;
    desc.position = pos;
    desc.team = AvailableTeams::enemy;
    const auto e = create_combat_entity(r, desc);

    room_idx_to_spawn++;
    room_idx_to_spawn %= rooms.size();
  }
};

std::pair<bool, std::optional<Room>>
inside_room(const MapComponent& map, const std::vector<Room>& rooms, const glm::ivec2& gridpos)
{
  const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };

  // create aabb from gridpos
  // const auto worldpos = gridpos * map.tilesize;
  // const auto worldpos_center = worldpos + offset;

  AABB aabb;
  aabb.center = gridpos;
  aabb.size = { glm::abs(1), glm::abs(1) };

  for (const auto& room : rooms) {
    // // create aabb for room
    // AABB room_worldspace_aabb;
    // room_worldspace_aabb.center = room.aabb.center * map.tilesize;
    // room_worldspace_aabb.center += offset;
    // room_worldspace_aabb.size = room.aabb.size * map.tilesize;

    // Room AABB is in gridspace
    if (collide(room.aabb, aabb))
      return { true, room };
  }

  return { false, std::nullopt };
};

bool
inside_tunnel(const std::vector<Tunnel>& ts, const glm::ivec2& gridpos)
{
  for (const auto& t : ts) {
    for (const auto& l : t.line_0) {
      if (l.first == gridpos.x && l.second == gridpos.y)
        return true;
    }
    for (const auto& l : t.line_1) {
      if (l.first == gridpos.x && l.second == gridpos.y)
        return true;
    }
  }
  return false;
};

void
instantiate_tunnels(entt::registry& r, std::vector<Line>& lines, DungeonGenerationResults& results)
{
  const auto& map = get_first_component<MapComponent>(r);

  // Debug Tunnel Lines
  // for (const Tunnel& t : results.tunnels) {
  //   const auto two_tunnel_lines = convert_tunnel_to_lines(r, map, t);
  //   for (const auto& t_line : two_tunnel_lines) {
  //     const auto shrunk_line = shrink_line_at_each_end(t_line, 5);
  //     const auto e = create_wall(r, get_center_from_line(shrunk_line), get_size_from_line(shrunk_line));
  //     set_colour(r, e, { 1.0f, 0.0, 0.0, 1.0f });
  //     r.get<TagComponent>(e).tag = "tunnel-line";
  //   }
  // }

  const auto create_walls_based_on_neighbours = [&r, &map, &results](const glm::ivec2& gp) {
    const auto idxs = engine::grid::get_neighbour_indicies(gp.x, gp.y, map.xmax, map.ymax);
    for (const auto& [dir, neighbour_idx] : idxs) {

      // dont create walls at floors
      const auto neighbour_is_floor = results.wall_or_floors[neighbour_idx] == 0;
      if (neighbour_is_floor)
        continue;

      // dont create walls at rooms
      // although possibly this is where to create doors
      const auto neighbour_gp = engine::grid::index_to_grid_position(neighbour_idx, map.xmax, map.ymax);
      const auto [in_room, room] = inside_room(map, results.rooms, neighbour_gp);
      if (in_room)
        continue;

      // else: neighbour is a wall, create a wall
      const auto& worldspace_tl = gp * map.tilesize;
      const auto worldspace_size = glm::ivec2(map.tilesize);
      const int w = worldspace_size.x;
      const int h = worldspace_size.y;

      // const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
      const glm::ivec2 offset = { 0, 0 };

      const glm::ivec2 tl = glm::ivec2{ worldspace_tl } + offset;
      const glm::ivec2 tr = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y) } + offset;
      const glm::ivec2 bl = glm::ivec2{ (worldspace_tl.x), (worldspace_tl.y + h) } + offset;
      const glm::ivec2 br = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y + h) } + offset;
      const glm::ivec2 worldspace_center = { (tl.x + tr.x) / 2.0f, (tr.y + br.y) / 2.0f };

      Line l;
      // dir is the dir to create a wall
      if (dir == engine::grid::GridDirection::north)
        l = { tl, tr };
      if (dir == engine::grid::GridDirection::south)
        l = { bl, br };
      if (dir == engine::grid::GridDirection::east)
        l = { tr, br };
      if (dir == engine::grid::GridDirection::west)
        l = { tl, bl };

      create_wall(r, get_center_from_line(l), get_size_from_line(l));

      // keep a record of the tunnel line that was created
      results.lines_to_instantiate.push_back(l);
    }
  };

  // Fill up gaps in space...
  for (const Tunnel& tunnel : results.tunnels) {
    const auto& square_0 = tunnel.line_0;
    const auto& square_1 = tunnel.line_1;

    for (const std::pair<int, int>& gridpos : square_0) {
      const auto [has_coll, room] = inside_room(map, results.rooms, { gridpos.first, gridpos.second });
      if (has_coll)
        continue;
      create_walls_based_on_neighbours({ gridpos.first, gridpos.second });
    }

    for (const std::pair<int, int>& gridpos : square_1) {
      const auto [has_coll, room] = inside_room(map, results.rooms, { gridpos.first, gridpos.second });
      if (has_coll)
        continue;
      create_walls_based_on_neighbours({ gridpos.first, gridpos.second });
    }

    // give tunnel representation to entt
    for (const Tunnel& t : results.tunnels)
      create_empty<Tunnel>(r, t);
  }
};

void
generate_edges(entt::registry& r, MapComponent& map, const DungeonGenerationResults& result)
{
  std::vector<Room> rooms;
  for (const auto& [e, room] : r.view<Room>().each())
    rooms.push_back(room);

  std::vector<Tunnel> tunnels;
  for (const auto& [e, t] : r.view<Tunnel>().each())
    tunnels.push_back(t);

  map.edges.clear();
  for (int y = 0; y < map.ymax - 1; y++) {
    for (int x = 0; x < map.xmax - 1; x++) {
      const auto idx = engine::grid::grid_position_to_index({ x, y }, map.xmax);
      const auto neighbours = engine::grid::get_neighbour_indicies(x, y, map.xmax, map.ymax);

      for (const auto& [dir, neighbour_idx] : neighbours) {
        const auto grid_a = engine::grid::index_to_grid_position(idx, map.xmax, map.ymax);
        const auto grid_b = engine::grid::index_to_grid_position(neighbour_idx, map.xmax, map.ymax);

        // 3 cases to generate edges for walls:
        // transition from wall to floor
        // transition from room to room
        // transition from room to tunnel

        const bool idx_is_wall = result.wall_or_floors[idx] == 1;
        const bool nidx_is_floor = result.wall_or_floors[neighbour_idx] == 0;
        const bool from_wall_to_floor = idx_is_wall && nidx_is_floor;

        const auto [in_room_a, room_a] = inside_room(map, rooms, grid_a);
        const auto [in_room_b, room_b] = inside_room(map, rooms, grid_b);
        const auto in_tunnel_a = inside_tunnel(tunnels, grid_a);
        const auto in_tunnel_b = inside_tunnel(tunnels, grid_b);

        const bool from_room_to_room =
          in_room_a && in_room_b && (!in_tunnel_a || !in_tunnel_b) && room_a.value() != room_b.value();

        bool from_room_to_tunnel = in_room_a && in_tunnel_b;
        from_room_to_tunnel &= !(in_room_a && in_room_b); // dont generate edge in same room
        // from_room_to_tunnel &= !(in_tunnel_a && in_tunnel_b); // dont generate edge if both are tunnels

        if (from_wall_to_floor || from_room_to_room || from_room_to_tunnel) {
          Edge edge;
          edge.cell_a = grid_a;
          edge.cell_b = grid_b;
          map.edges.push_back(edge);
        }
      }
    }
  }

  fmt::println("Map edges: {}", map.edges.size());
};

} // namespace game2d
