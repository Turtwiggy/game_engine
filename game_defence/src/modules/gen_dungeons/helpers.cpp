#include "helpers.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "colour/colour.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "helpers/line.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/gen_dungeons/helpers/collisions.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/system_drop_items/components.hpp"

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
          // bool create_as_wall = false;
          // if (x == 0 || x == w - 1)
          //   create_as_wall = true;
          // if (y == 0 || y == h - 1)
          //   create_as_wall = true;

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
  DataSolidWall desc;
  desc.pos = pos;
  desc.size = size;
  desc.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  return Factory_DataSolidWall::create(r, desc);
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
  // const glm::ivec2 bl = glm::ivec2{ (worldspace_tl.x), (worldspace_tl.y + h) };
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

// Split the wall in to multiple walls per tilesize
void
instantiate_segments_from_line(entt::registry& r, const glm::ivec2& size, const glm::ivec2& center)
{
  const auto& map = get_first_component<MapComponent>(r);

  const bool is_horizontal = size.x > size.y;

  // if ((is_horizontal && size.x < map.tilesize) || (!is_horizontal && size.y < map.tilesize))
  //   return;

  const int chunks = is_horizontal ? size.x / map.tilesize : size.y / map.tilesize;
  if (chunks <= 0)
    return;
  const glm::vec2 chunk_size = is_horizontal ? glm::vec2{ size.x / chunks, size.y } : glm::vec2{ size.x, size.y / chunks };
  const auto chunk_l = is_horizontal ? center.x - (chunk_size.x * (chunks / 2.0f)) : center.x;
  const auto chunk_t = is_horizontal ? center.y : center.y - (chunk_size.y * (chunks / 2.0f));
  const auto chunk_tl = glm::vec2{ chunk_l, chunk_t };
  for (int i = 0; i < chunks; i++) {
    const float pos_x = is_horizontal ? chunk_tl.x + i * chunk_size.x + map.tilesize / 2.0f : chunk_tl.x;
    const float pos_y = is_horizontal ? chunk_tl.y : chunk_tl.y + i * chunk_size.y + map.tilesize / 2.0f;
    create_wall(r, { pos_x, pos_y }, chunk_size);
  }
}

void
set_generated_entity_positions(entt::registry& r, DungeonGenerationResults& results, engine::RandomState& rnd)
{
  const auto& map_c = get_first_component<MapComponent>(r);
  // const auto& data = get_first_component<OverworldToDungeonInfo>(r);
  // const int strength = data.patrol_that_you_hit.strength;
  const int strength = 10; // TODO: fix this
  auto& rooms = results.rooms;

  const int max_attempts_to_generate_unique_spot = 3;
  // const int potential_items_per_room = 5 + (floor);
  // const int potential_monsters_per_room = 5 + (floor);
  // printf("floor %i has max items %i, max monsters: %i\n", floor, potential_items_per_room, potential_monsters_per_room);

  int room_idx_to_spawn = 1;
  for (int i = strength; i > 0; i--) {
    if (room_idx_to_spawn == 0)
      room_idx_to_spawn++; // first index room is player room

    if (rooms.size() < 2) {
      fmt::println("WARNING! roomgen issue...");
      break;
    }
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

    // actor_dungeon description

    DataDungeonActor desc;
    desc.pos = pos;
    desc.team = AvailableTeams::enemy;
    desc.hp = 50;
    desc.max_hp = 50;
    desc.hovered_colour = { 1.0f, 0.0f, 0.0f, 1.0f };
    const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);

    const auto idx = engine::grid::worldspace_to_index(pos, map_c.tilesize, map_c.xmax, map_c.ymax);
    add_entity_to_map(r, dungeon_e, idx);

    // get enemies to drop scrap
    r.emplace<DropItemsOnDeathComponent>(dungeon_e);

    // give enemies a shotgun
    DataWeaponShotgun wdesc;
    wdesc.able_to_shoot = true;
    wdesc.parent = dungeon_e;
    wdesc.team = desc.team;
    const auto weapon_e = Factory_DataWeaponShotgun::create(r, wdesc);

    room_idx_to_spawn++;
    room_idx_to_spawn %= rooms.size();
  }
};

//

std::pair<bool, std::optional<Room>>
inside_room(const MapComponent& map, const std::vector<Room>& rooms, const glm::ivec2& gridpos)
{
  RoomAABB aabb;
  aabb.center = gridpos;
  aabb.size = { glm::abs(1), glm::abs(1) };

  for (const Room& room : rooms) {
    // Room AABB is in gridspace
    if (collide(room.aabb, aabb))
      return { true, room };
  }

  return { false, std::nullopt };
};

std::vector<Tunnel>
inside_tunnels(const std::vector<Tunnel>& ts, const glm::ivec2& gridpos)
{
  std::vector<Tunnel> ts_results;

  for (const auto& t : ts) {

    for (const auto& l : t.line_0)
      if (l.first == gridpos.x && l.second == gridpos.y)
        ts_results.push_back(t);

    for (const auto& l : t.line_1)
      if (l.first == gridpos.x && l.second == gridpos.y)
        ts_results.push_back(t);
  }

  return ts_results;
};

void
remove_line(std::vector<Line>& lines, const Line& line)
{
  std::erase(lines, line);
};

void
instantiate_walls(entt::registry& r, DungeonGenerationResults& results)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto& tunnels = results.tunnels;
  const float half_tilesize = map.tilesize / 2.0f;

  std::vector<Line> wall_lines_to_instantiate;

  for (const Room& room : results.rooms) {
    // create room walls
    const auto& gridspace_tl = room.tl;
    const auto& worldspace_tl = gridspace_tl * map.tilesize;
    const auto worldspace_size = room.aabb.size * map.tilesize;

    const int w = worldspace_size.x;
    const int h = worldspace_size.y;
    const glm::ivec2 tl = glm::ivec2{ worldspace_tl };
    const glm::ivec2 tr = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y) };
    const glm::ivec2 bl = glm::ivec2{ (worldspace_tl.x), (worldspace_tl.y + h) };
    const glm::ivec2 br = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y + h) };
    const glm::ivec2 worldspace_center = { (tl.x + tr.x) / 2.0f, (tr.y + br.y) / 2.0f };

    // Convert room to 4 lines
    const Line l0{ tl, tr }; // horizontal
    const Line l1{ tr, br }; // vertical
    const Line l2{ bl, br }; // horizontal
    const Line l3{ tl, bl }; // vertical

    const int num_lines_prior = (int)wall_lines_to_instantiate.size();
    wall_lines_to_instantiate.push_back(l0);
    wall_lines_to_instantiate.push_back(l1);
    wall_lines_to_instantiate.push_back(l2);
    wall_lines_to_instantiate.push_back(l3);

    // now shrink the walls based on tunnels
    //
    // A tunnel consists of 2 lines.
    for (const Tunnel& t : tunnels) {
      for (const Line& t_line : convert_tunnel_to_lines(r, map, t)) {

        // shrink line so it doesnt collide at ends
        // check if the tunnel line collides with any of the wall lines
        const Line t_line_shrunk = shrink_line_at_each_end(t_line, 5);
        const std::vector<LineCollision> collisions = get_line_collisions(wall_lines_to_instantiate, t_line_shrunk);
        for (const auto& coll_info : collisions) {
          Line wall_line = coll_info.line_a;
          // Line tunnel_line = coll_info.line_b;
          const auto intersection = coll_info.intersection;

          // remove the wall if it collided
          remove_line(wall_lines_to_instantiate, wall_line);

          make_p1_bigger_point(wall_line);

          // split the line in to 2 lines...
          Line new_line_a;
          new_line_a.p0 = wall_line.p0;
          new_line_a.p1 = intersection;

          Line new_line_b;
          new_line_b.p0 = intersection;
          new_line_b.p1 = wall_line.p1;

          if (new_line_a.p0 == new_line_a.p1)
            fmt::println("(warning: map gen). Weird collision. Debug this.");
          if (new_line_b.p0 == new_line_b.p1)
            fmt::println("(warning: map gen). Weird collision. Debug this.");
          if (new_line_a.p0.x != new_line_a.p1.x && new_line_a.p0.y != new_line_a.p1.y)
            fmt::println("(warning: map gen). Diagonal line. Debug this.");
          if (new_line_b.p0.x != new_line_b.p1.x && new_line_b.p0.y != new_line_b.p1.y)
            fmt::println("(warning: map gen). Diagonal line. Debug this.");

          // Adjust the new two new lines so that they're half a grid apart
          if (is_horizontal(wall_line)) {
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
            wall_lines_to_instantiate.push_back((new_line_a));
          if (line_is_valid(new_line_b))
            wall_lines_to_instantiate.push_back((new_line_b));

          //
        }
      }
    }

    const int num_lines_post = (int)wall_lines_to_instantiate.size();
    if (num_lines_post - num_lines_prior < 4) // 4 because a 2d room has 4 walls
      fmt::println("error: did not add enough lines for room to be covered.");
  }

  // Instantiate Room lines
  // for (const auto& l : wall_lines_to_instantiate) {
  //   const auto size = get_size_from_line(l);
  //   const auto center = get_center_from_line(l);
  //   instantiate_segments_from_line(r, size, center);
  // }
};

void
instantiate_tunnels(entt::registry& r, DungeonGenerationResults& results)
{
  const auto& map = get_first_component<MapComponent>(r);

  // Debug Tunnel Lines
  // for (const Tunnel& t : results.tunnels) {
  //   const auto two_tunnel_lines = convert_tunnel_to_lines(r, map, t);
  //   for (const auto& t_line : two_tunnel_lines) {
  //     const auto shrunk_line = shrink_line_at_each_end(t_line, 5);
  //     const auto e = create_wall(r, get_center_from_line(shrunk_line), get_size_from_line(shrunk_line));
  //     set_colour(r, e, { 0.0f, 0.0, 1.0, 1.0f });
  //     r.get<TagComponent>(e).tag = "tunnel-line";
  //   }
  // }

  const auto create_walls_based_on_neighbours = [&r, &map, &results](const glm::ivec2& gp) {
    const auto idxs = engine::grid::get_neighbour_indicies(gp.x, gp.y, map.xmax, map.ymax);
    for (const auto& [dir, neighbour_idx] : idxs) {
      const auto neighbour_gp = engine::grid::index_to_grid_position(neighbour_idx, map.xmax, map.ymax);

      // dont create walls at rooms
      // although possibly this is where to create doors
      const auto [in_room, room] = inside_room(map, results.rooms, neighbour_gp);
      // const bool create_as_door = results.wall_or_floors[idx] == 0 && in_room;
      const bool create_as_door = false;

      // dont create walls at floors
      const auto neighbour_is_floor = results.wall_or_floors[neighbour_idx] == 0;
      if (neighbour_is_floor && !create_as_door)
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

      const auto size = get_size_from_line(l);
      const auto center = get_center_from_line(l);
      const auto e = create_wall(r, center, size);

      if (create_as_door)
        set_colour(r, e, { 1.0f, 0.0f, 0.0f, 1.0f });

      // keep a record of the tunnel line that was created
      // results.lines_to_instantiate.push_back(l);
    }
  };

  // For all the tunnels, iterate along all the grid-cells
  //
  for (const Tunnel& tunnel : results.tunnels) {

    for (const std::pair<int, int>& gridpos : tunnel.line_0) {
      const auto [has_coll, room] = inside_room(map, results.rooms, { gridpos.first, gridpos.second });
      if (has_coll)
        continue;
      create_walls_based_on_neighbours({ gridpos.first, gridpos.second });
    }

    for (const std::pair<int, int>& gridpos : tunnel.line_1) {
      const auto [has_coll, room] = inside_room(map, results.rooms, { gridpos.first, gridpos.second });
      if (has_coll)
        continue;
      create_walls_based_on_neighbours({ gridpos.first, gridpos.second });
    }
  }
};

void
generate_edges(entt::registry& r, MapComponent& map, const DungeonGenerationResults& result)
{
  const auto& rooms = result.rooms;
  const auto& tunnels = result.tunnels;

  map.edges.clear();

  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    const auto xy = engine::grid::index_to_grid_position(idx, map.xmax, map.ymax);
    const auto neighbours = engine::grid::get_neighbour_indicies(xy.x, xy.y, map.xmax, map.ymax);
    for (const auto& [dir, neighbour_idx] : neighbours) {

      const auto grid_a = engine::grid::index_to_grid_position(idx, map.xmax, map.ymax);
      const auto grid_b = engine::grid::index_to_grid_position(neighbour_idx, map.xmax, map.ymax);

      // 3 cases to generate edges for walls:
      // transition from wall to floor
      // transition from room to room
      // transition from room to tunnel
      // you're in a room and on the edge of the grid

      const bool idx_is_wall = result.wall_or_floors[idx] == 1;
      const bool nidx_is_floor = result.wall_or_floors[neighbour_idx] == 0;
      const bool from_wall_to_floor = idx_is_wall && nidx_is_floor;

      const auto [in_room_a, room_a] = inside_room(map, rooms, grid_a);
      const auto [in_room_b, room_b] = inside_room(map, rooms, grid_b);
      const auto tunnels_a = inside_tunnels(tunnels, grid_a);
      const auto tunnels_b = inside_tunnels(tunnels, grid_b);
      // const bool in_tunnel_a = tunnels_a.size() > 0;
      const bool in_tunnel_b = tunnels_b.size() > 0;

      // same_tunnel means:
      // if two gridsquares are in the same_tunnel,
      // that means there is a direct path from a to b (i.e. no edges)
      bool same_tunnel = false;
      for (const auto& t_a : tunnels_a)
        for (const auto& t_b : tunnels_b)
          same_tunnel |= t_a == t_b;

      const bool from_room_to_room = (in_room_a && in_room_b) && (room_a.value() != room_b.value());

      const bool from_room_to_tunnel = in_room_a && in_tunnel_b && !in_room_b;

      Edge edge;
      edge.a_idx = engine::grid::grid_position_to_index(grid_a, map.xmax);
      edge.b_idx = engine::grid::grid_position_to_index(grid_b, map.xmax);
      if (edge.a_idx > edge.b_idx)
        std::swap(edge.a_idx, edge.b_idx);

      if (from_wall_to_floor)
        map.edges.push_back(edge);
      else if (from_room_to_room && !same_tunnel)
        map.edges.push_back(edge);
      else if (from_room_to_tunnel && !same_tunnel)
        map.edges.push_back(edge);

      // check if you're in a room and on the edge of the grid
      const bool on_x_min = xy.x == 0;
      const bool on_y_min = xy.y == 0;
      const bool on_x_max = xy.x == map.xmax;
      const bool on_y_max = xy.y == map.ymax;
      const bool add_boundary = (on_x_min || on_y_min || on_x_max || on_y_max) && in_room_a;
      Edge b_edge;
      b_edge.a_idx = engine::grid::grid_position_to_index(grid_a, map.xmax);
      b_edge.b_idx = -1; // off the grid... what value would make sense?
      if (add_boundary)
        map.edges.push_back(b_edge);
    }
  }

  // remove duplicates
  // std::unique only removes consecutive duplicates
  auto p = [](const Edge& a, const Edge& b) { return (a.a_idx < b.a_idx) || (a.a_idx == b.a_idx && a.b_idx < b.b_idx); };
  std::sort(map.edges.begin(), map.edges.end(), p);

  auto pred = [](const Edge& a, const Edge& b) { return a.a_idx == b.a_idx && a.b_idx == b.b_idx; };
  map.edges.erase(std::unique(map.edges.begin(), map.edges.end(), pred), map.edges.end());

  fmt::println("Map edges: {}", map.edges.size());
};

void
instantiate_edges(entt::registry& r, MapComponent& map)
{
  for (auto& edge : map.edges) {
    const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
    const auto ga = engine::grid::index_to_world_position(edge.a_idx, map.xmax, map.ymax, map.tilesize) + offset;
    glm::vec2 gb{ 0, 0 };

    // handle off the grid case...
    if (edge.b_idx == -1) {

      // convert invalid index to grid position
      const glm::ivec2 xy = engine::grid::index_to_grid_position(edge.a_idx, map.xmax, map.ymax);
      glm::ivec2 gp{ 0, 0 };
      if (xy.x == 0)
        gp = { -1, xy.y };
      if (xy.y == 0)
        gp = { xy.x, -1 };
      if (xy.x == map.xmax)
        gp = { xy.x + 1, xy.y };
      if (xy.y == map.ymax)
        gp = { xy.x, xy.y + 1 };

      gb = engine::grid::grid_space_to_world_space(gp, map.tilesize) + offset;
    } else
      gb = engine::grid::index_to_world_position(edge.b_idx, map.xmax, map.ymax, map.tilesize) + offset;

    // debug edge
    // const auto l0 = generate_line({ ga.x, ga.y }, { gb.x, gb.y }, 4);
    // const entt::entity e0 = create_gameplay(r, EntityType::empty_with_transform, { 0, 0 });
    // set_position_and_size_with_line(r, e0, l0);
    // set_colour(r, e0, { 1.0f, 0.0, 0.0, 0.5f });

    const auto center = (gb + ga) / 2.0f;
    const auto size = glm::abs(gb - ga);
    const bool was_horizontal = size.x > size.y;

    auto new_size = glm::vec2{ size.y, size.x };
    if (new_size.x == 0)
      new_size.x = 2;
    if (new_size.y == 0)
      new_size.y = 2;

    if (edge.instance != entt::null) {
      fmt::println("Warning: edge already had instance...");
      continue;
    }

    DataSolidWall desc;
    desc.pos = center;
    desc.size = new_size;
    const auto e = Factory_DataSolidWall::create(r, desc);

    // if (was_horizontal)
    //   set_colour(r, e, { 0.0f, 1.0, 0.0, 1.0f });
    // else
    //   set_colour(r, e, { 0.0f, 0.0, 1.0, 1.0f });

    if (edge.instance != entt::null)
      fmt::println("Warning: edge already had instance...");

    edge.instance = e;
  }

  // validate...
  for (const auto& edge : map.edges) {
    if (edge.instance == entt::null)
      fmt::println("Warning: edge without instance?");
  }
}

} // namespace game2d
