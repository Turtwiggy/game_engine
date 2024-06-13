#include "helpers.hpp"

#include "actors.hpp"
#include "colour/colour.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "helpers/line.hpp"
#include "imgui.h"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/transform.hpp"

#include <algorithm>
#include <iostream>
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
    std::cout << "gen room. size: " << w << ", " << h << " at " << room.tl.x << ", " << room.tl.y << std::endl;

    // update_wall_of_floors_bitmap
    auto create_room = [](const Room& room, std::vector<int>& wall_or_floors, const int xmax) {
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
instantiate_walls(entt::registry& r, std::vector<Line>& lines, const DungeonGenerationResults& results, const int& i)
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
    const Line l0{ tl, tr, 0 }; // horizontal
    const Line l1{ tr, br, 1 }; // vertical
    const Line l2{ bl, br, 2 }; // horizontal
    const Line l3{ tl, bl, 3 }; // vertical

    // only add these lines to lines_to_instantiate if there is no collision
    // const auto coll0 = get_line_collisions(lines_to_instantiate, l0);
    // const auto coll1 = get_line_collisions(lines_to_instantiate, l1);
    // const auto coll2 = get_line_collisions(lines_to_instantiate, l2);
    // const auto coll3 = get_line_collisions(lines_to_instantiate, l3);

    const int num_lines_prior = lines_to_instantiate.size();
    lines_to_instantiate.push_back(l0);
    lines_to_instantiate.push_back(l1);
    lines_to_instantiate.push_back(l2);
    lines_to_instantiate.push_back(l3);

    // There WILL be a tunnel(s) going out of this room.
    // std::vector<Tunnel> tunnels_of_interest;
    // for (const auto& t : tunnels)
    //   if (t.room == room || t.prev_room == room)
    //     tunnels_of_interest.push_back(t);

    if (room_count == 3)
      int k = 1; // debug specific room

    // A tunnel consists of 2 lines.
    for (const Tunnel& t : tunnels) {
      const auto two_tunnel_lines = convert_tunnel_to_lines(r, map, t);
      for (const auto& t_line : two_tunnel_lines) {

        // shrink line so it doesnt collide at ends
        const Line t_line_shrunk = shrink_line_at_each_end(t_line, 5);

        const auto collisions = get_line_collisions(lines_to_instantiate, t_line_shrunk);
        if (collisions.size() > 0)
          int k = 1;

        // Match tunnel collisions to lines_to_instantiate walls.
        for (const auto& [coll_line, coll_intersection] : collisions) {

          if (coll_line.p0.x == 650 && coll_line.p0.y == 50)
            int k = 1;

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
          new_line_a.room_debug_dir = 999;
          new_line_a.room_debug_idx = room_count;

          Line new_line_b;
          new_line_b.p0 = coll_intersection;
          new_line_b.p1 = line_to_split.p1;
          new_line_b.room_debug_dir = 999;
          new_line_b.room_debug_idx = room_count;

          if (new_line_a.p0 == new_line_a.p1)
            std::cout << "Weird collision. Debug this." << std::endl;
          if (new_line_b.p0 == new_line_b.p1)
            std::cout << "Weird collision. Debug this." << std::endl;
          if (new_line_a.p0.x != new_line_a.p1.x && new_line_a.p0.y != new_line_a.p1.y)
            std::cout << "Diagonal line. Debug this." << std::endl;
          if (new_line_b.p0.x != new_line_b.p1.x && new_line_b.p0.y != new_line_b.p1.y)
            std::cout << "Diagonal line. Debug this." << std::endl;

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
    if (num_lines_post - num_lines_prior < 4) {
      std::cout << "error: did not add enough lines for room to be covered." << std::endl;
      int k = 1;
    }

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
    r.emplace<WorldspaceTextComponent>(room_to_create, std::to_string(room_count));

    room_count++;
  }

  // Instantiate lines
  for (const auto& l : lines_to_instantiate) {
    const auto e = create_wall(r, get_center_from_line(l), get_size_from_line(l));
    set_colour(r, e, { 0.0f, 1.0f, 0.0f, 1.0f });
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

std::pair<bool, std::optional<AABB>>
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
      return { true, room.aabb };
  }

  return { false, std::nullopt };
};

void
instantiate_tunnels(entt::registry& r, std::vector<Line>& lines, const DungeonGenerationResults& results)
{
  const auto& map = get_first_component<MapComponent>(r);

  // for (const auto& tunnel : results.tunnels) {
  //   const auto create_square = [&r, &map, &lines](const std::pair<int, int>& a, const std::pair<int, int>& b) {
  //     // const auto tunnel_to_create = create_gameplay(r, EntityType::empty_with_transform);
  //     // const engine::SRGBColour col = { 1.0f, 0.0f, 0.0f, 0.1f };
  //     // set_colour(r, tunnel_to_create, col);
  //     // r.emplace<DefaultColour>(tunnel_to_create, col);

  //     const int grid_w = glm::abs(b.first - a.first) + 1;
  //     const int grid_h = glm::abs(b.second - a.second) + 1;
  //     const glm::ivec2 gridspace_tl = { glm::min(a.first, b.first), glm::min(a.second, b.second) };

  //     const auto& worldspace_tl = gridspace_tl * map.tilesize;
  //     const auto worldspace_size = glm::ivec2{ grid_w, grid_h } * map.tilesize;
  //     const int w = worldspace_size.x;
  //     const int h = worldspace_size.y;

  //     // const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
  //     const glm::ivec2 offset = { 0, 0 };

  //     const glm::ivec2 tl = glm::ivec2{ worldspace_tl } + offset;
  //     const glm::ivec2 tr = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y) } + offset;
  //     const glm::ivec2 bl = glm::ivec2{ (worldspace_tl.x), (worldspace_tl.y + h) } + offset;
  //     const glm::ivec2 br = glm::ivec2{ (worldspace_tl.x + w), (worldspace_tl.y + h) } + offset;
  //     const glm::ivec2 worldspace_center = { (tl.x + tr.x) / 2.0f, (tr.y + br.y) / 2.0f };

  //     const Line l0{ tl, tr };
  //     const Line l1{ tr, br };
  //     const Line l2{ br, bl };
  //     const Line l3{ bl, tl };
  //     const auto e0 = create_wall(r, get_center_from_line(l0), get_size_from_line(l0, 1));
  //     const auto e1 = create_wall(r, get_center_from_line(l1), get_size_from_line(l1, 1));
  //     const auto e2 = create_wall(r, get_center_from_line(l2), get_size_from_line(l2, 1));
  //     const auto e3 = create_wall(r, get_center_from_line(l3), get_size_from_line(l3, 1));

  //     set_colour(r, e0, { 1.0f, 0.0, 0.0, 1.0f });
  //     set_colour(r, e1, { 1.0f, 0.0, 0.0, 1.0f });
  //     set_colour(r, e2, { 1.0f, 0.0, 0.0, 1.0f });
  //     set_colour(r, e3, { 1.0f, 0.0, 0.0, 1.0f });
  //   };
  //   const auto& square_0 = tunnel.line_0;
  //   const auto& square_1 = tunnel.line_1;
  //   if (square_0.size() >= 1) {               // first can be last
  //     const auto square_pFirst = square_0[0]; // could be horizontal or vertical
  //     const auto square_pLast = square_0[square_0.size() - 1];
  //     create_square(square_pFirst, square_pLast);
  //   }
  //   if (square_1.size() >= 1) {
  //     const auto square_pFirst = square_1[0]; // could be horizontal or vertical
  //     const auto square_pLast = square_1[square_1.size() - 1];
  //     create_square(square_pFirst, square_pLast);
  //   }
  //   r.emplace<Tunnel>(r.create(), tunnel);

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

  // Fill up gaps in space...
  for (const Tunnel& tunnel : results.tunnels) {
    const auto& square_0 = tunnel.line_0;
    const auto& square_1 = tunnel.line_1;

    for (const std::pair<int, int>& gridpos : square_0) {
      const auto [coll, coll_aabb] = inside_room(map, results.rooms, { gridpos.first, gridpos.second });
      // if (coll)
      //   continue;
      const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
      const auto worldpos = glm::ivec2{ gridpos.first, gridpos.second } * map.tilesize;
      const auto worldpos_center = worldpos + offset;
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      set_position(r, e, worldpos_center);
      set_size(r, e, { 5, 5 });
    }

    for (const std::pair<int, int>& gridpos : square_1) {
      const auto [coll, coll_aabb] = inside_room(map, results.rooms, { gridpos.first, gridpos.second });
      // if (coll)
      //   continue;
      const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
      const auto worldpos = glm::ivec2{ gridpos.first, gridpos.second } * map.tilesize;
      const auto worldpos_center = worldpos + offset;
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      set_position(r, e, worldpos_center);
      set_size(r, e, { 5, 5 });
    }
  }

  for (const Tunnel& t : results.tunnels)
    r.emplace<Tunnel>(r.create(), t);
};

} // namespace game2d
