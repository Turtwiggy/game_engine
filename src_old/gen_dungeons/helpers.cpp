#include "helpers.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "helpers/line.hpp"
#include "modules/actor_door/components.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/priority_queue.hpp"
#include "modules/gen_dungeons/helpers/collisions.hpp"
#include "modules/grid/components.hpp"


#include <algorithm>
#include <fmt/core.h>
#include <vector>

namespace game2d {

// i.e. take all the isolated islands of floor tiles and convert them in to the room representation
[[nodiscard]] std::vector<int>
convert_floor_islands_to_room(entt::registry& r,
                              const MapComponent& map_c,
                              const int from_idx,
                              const DungeonGenerationResults& dresults)
{
  std::vector<astar_cell> map = generate_map_view(r, map_c);
  map[from_idx].distance = 0;

  const auto from_glm = engine::grid::index_to_grid_position(from_idx, map_c.xmax, map_c.ymax);
  const vec2i from{ from_glm.x, from_glm.y };

  PriorityQueue<vec2i> frontier;
  frontier.enqueue(from, 0);

  std::vector<int> results;

  while (frontier.size() > 0) {
    const auto current = frontier.dequeue();
    const auto current_idx = engine::grid::grid_position_to_index({ current.x, current.y }, map_c.xmax);

    const bool is_floor = dresults.wall_or_floors[current_idx] == 0;
    if (!is_floor)
      continue; // starting tile isnt floor
    const auto rooms = inside_room(r, { current.x, current.y });
    if (rooms.size() > 0)
      continue; // gone to a different room

    results.push_back(current_idx);

    // check neighbours
    const auto neighbours_idxs = engine::grid::get_neighbour_indicies(current.x, current.y, map_c.xmax, map_c.ymax);

    for (const auto& [dir, idx] : neighbours_idxs) {
      astar_cell& neighbour = map[idx];

      const bool n_is_floor = dresults.wall_or_floors[idx] == 0;
      if (!n_is_floor)
        continue;
      const auto rooms = inside_room(r, { current.x, current.y });
      if (rooms.size() > 0)
        continue; // gone to a different room

      int distance = map[current_idx].distance + 1;

      if (neighbour.distance == INT_MAX) {
        neighbour.distance = distance;
        frontier.enqueue(neighbour.pos, 0);
      } else if (distance < neighbour.distance) {
        neighbour.distance = distance;
      }
    }
  }

  return results;
};

void
convert_tunnels_to_rooms(entt::registry& r, const DungeonGenerationResults& results)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  for (int xy = 0; xy < map_c.xmax * map_c.ymax; xy++) {

    std::vector<int> room_idxs = convert_floor_islands_to_room(r, map_c, xy, results);
    if (room_idxs.size() == 0)
      continue;
    fmt::println("connected room found of size: {}", room_idxs.size());

    Room room;
    room.tiles_idx = room_idxs;
    create_empty<Room>(r, room);
  }
};

std::vector<int>
convert_square_room_to_idx(Room& room, const MapComponent& map_c)
{
  std::vector<int> results;

  const glm::ivec2 tl = room.tl.value();
  const glm::ivec2 br = room.tl.value() + room.aabb.value().size;

  for (int x = tl.x; x < br.x; x++)
    for (int y = tl.y; y < br.y; y++)
      results.push_back(engine::grid::grid_position_to_index({ x, y }, map_c.xmax));

  return results;
};

void
generate_tunnel(entt::registry& r, const Room a, const Room b, DungeonGenerationResults& result)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  const auto a_center = a.aabb.value().center;
  const auto b_center = b.aabb.value().center;
  const auto dir = engine::normalize_safe(b_center - a_center);

  int x1 = a_center.x;
  int y1 = a_center.y;
  int x2 = b_center.x;
  int y2 = b_center.y;

  int corner_x = 0;
  int corner_y = 0;

  // move vertically, then horizontally
  {
    corner_x = x1;
    corner_y = y2;
  }

  // generate lines via bresenham line algorithm
  const auto line_0 = create_line(x1, y1, corner_x, corner_y);
  const auto line_1 = create_line(corner_x, corner_y, x2, y2);

  // a) x1, y1 to corner_x, corner_y
  for (const auto& [x, y] : line_0) {
    result.wall_or_floors[engine::grid::grid_position_to_index({ x, y }, map_c.xmax)] = 0; // set as floor
  }

  // b) corner_x, corner_y to x2, y2
  for (const auto& [x, y] : line_1) {
    result.wall_or_floors[engine::grid::grid_position_to_index({ x, y }, map_c.xmax)] = 0; // set as floor
  }
  //
};

// create in-between rooms (i.e. tunnels or corridors) between rooms
// 1) set the tiles in the results to "floor"
// 2) create the sections outside the room as a new "room" and add to entt
void
connect_rooms_via_nearest_neighbour(entt::registry& r, DungeonGenerationResults& result)
{
  // start by obtaining the rooms list.
  std::vector<Room> rooms;
  for (const auto& [e, room_c] : r.view<const Room>().each())
    rooms.push_back(room_c);

  // make a new hashset name connected. we'll add rooms to this as they gain exits,
  // so as to avoid linking repeatedly to the same room.
  std::set<size_t> connected;

  for (size_t i = 0; i < rooms.size(); i++) {
    std::vector<std::pair<size_t, float>> room_idx_to_distance;

    const auto& room_c = rooms[i];
    const auto room_center = room_c.aabb.value().center;

    // work out the distance to all othe rooms.
    for (size_t j = 0; j < rooms.size(); j++) {
      const auto& room_c_other = rooms[j];
      if (room_c == room_c_other)
        continue;
      if (connected.contains(j))
        continue;
      const auto other_center = room_c_other.aabb.value().center;
      const glm::vec2 d = room_center - other_center;
      const float d2 = d.x * d.x + d.y * d.y;
      room_idx_to_distance.push_back({ j, d2 });
    }

    // sort by distance
    std::sort(room_idx_to_distance.begin(),
              room_idx_to_distance.end(),
              [](const std::pair<size_t, float>& a, const std::pair<size_t, float>& b) { return a.second < b.second; });

    if (room_idx_to_distance.size() == 0)
      continue;

    Room a = room_c;
    Room b = rooms[room_idx_to_distance[0].first];
    generate_tunnel(r, a, b, result);

    connected.emplace(i);
  }

  convert_tunnels_to_rooms(r, result);
};

DungeonGenerationResults
generate_rooms(entt::registry& r, const DungeonGenerationCriteria& data, engine::RandomState& rnd)
{
  const auto& map = get_first_component<MapComponent>(r);

  std::vector<int> wall_or_floors(map.xmax * map.ymax, 1); // 1: everything as wall

  const int max_rooms = data.max_rooms;
  const int room_size_min = data.room_size_min;
  const int room_size_max = data.room_size_max;
  const int map_width = map.xmax;
  const int map_height = map.ymax;
  const int xmax = map.xmax;

  std::vector<Room> rooms;

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
    RoomAABB aabb;
    aabb.center = { tl.x + (gen_room_width / 2), tl.y + (gen_room_height / 2) };
    aabb.size = { gen_room_width, gen_room_height };
    room.aabb = aabb;
    room.tiles_idx = convert_square_room_to_idx(room, map);

    // if new room collides with existin groom, skip this room.
    const auto room_collides = [&room](const Room& other) { return collide(room.aabb.value(), other.aabb.value()); };
    const auto it = std::find_if(rooms.begin(), rooms.end(), room_collides);
    if (it != rooms.end())
      continue;
    const auto& w = gen_room_width;
    const auto& h = gen_room_height;
    fmt::println("gen room. size: {}, {} at {}, {}", w, h, room.tl.value().x, room.tl.value().y);

    // update_wall_of_floors_bitmap
    auto create_room = [](const Room& room, std::vector<int>& wall_or_floors, const int xmax) {
      const auto& w = room.aabb.value().size.x;
      const auto& h = room.aabb.value().size.y;

      for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
          // by commenting out below,
          // set the entire generated room to "floor"
          const auto grid_pos = glm::ivec2{ room.tl.value().x + x, room.tl.value().y + y };
          const auto global_idx = engine::grid::grid_position_to_index(grid_pos, xmax);
          wall_or_floors[global_idx] = 0;
        }
      }
    };
    create_room(room, wall_or_floors, xmax);
    rooms.push_back(room);

  } // end iterating rooms

  DungeonGenerationResults results;
  results.rooms = rooms;
  results.wall_or_floors = wall_or_floors;

  // give to entt
  for (const Room& room : results.rooms)
    create_empty<Room>(r, room);

  return results;
};

std::vector<entt::entity>
inside_room(entt::registry& r, const glm::ivec2& gridpos)
{
  const auto& map_c = get_first_component<MapComponent>(r);
  const int idx = engine::grid::grid_position_to_index(gridpos, map_c.xmax);

  std::set<entt::entity> rooms;

  const auto& view = r.view<const Room>();

  for (const auto& [e, room_c] : view.each()) {
    const auto it = std::find(room_c.tiles_idx.begin(), room_c.tiles_idx.end(), idx);
    if (it != room_c.tiles_idx.end())
      rooms.emplace(e);
  }

  return { rooms.begin(), rooms.end() };
};

void
generate_edges(entt::registry& r, MapComponent& map, const DungeonGenerationResults& result)
{
  const auto& rooms = result.rooms;

  std::vector<Edge> edges;

  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    const auto xy = engine::grid::index_to_grid_position(idx, map.xmax, map.ymax);
    const auto neighbours = engine::grid::get_neighbour_gridpos(xy, map.xmax, map.ymax);
    for (const auto& [dir, neighbour_gp] : neighbours) {
      const auto grid_a = xy;
      const auto grid_b = neighbour_gp;

      // 3 cases to generate edges for walls:
      // transition from wall to floor
      // transition from room to room
      // you're in a room and on the edge of the grid

      Edge edge;
      edge.gp_a = grid_a;
      edge.gp_b = grid_b;

      const auto rooms_a = inside_room(r, grid_a);
      const auto rooms_b = inside_room(r, grid_b);
      const bool in_room_a = rooms_a.size() > 0;
      const bool in_room_b = rooms_b.size() > 0;
      const bool from_room_to_room = (in_room_a && in_room_b) && (rooms_a != rooms_b);

      // check if you're in a room and on the edge of the grid
      // grid_a has to be in bounds, as that's generated off the index
      // grid_b could be out of bounds, as that's the neighbour grid position
      bool in_bounds = engine::grid::grid_position_in_bounds(grid_b, map.xmax, map.ymax);
      if (!in_bounds && in_room_a) {
        edges.push_back(edge);
        continue;
      }

      if (!in_bounds)
        continue;
      const auto neighbour_idx = engine::grid::grid_position_to_index(neighbour_gp, map.xmax);

      const bool idx_is_wall = result.wall_or_floors[idx] == 1;
      const bool nidx_is_floor = result.wall_or_floors[neighbour_idx] == 0;
      const bool from_wall_to_floor = idx_is_wall && nidx_is_floor;

      if (from_wall_to_floor)
        edges.push_back(edge);

      // doors...
      // if (from_room_to_room)
      //   edges.push_back(edge);
    }
  }

  // std::sort for std::unique
  const auto p = [&map](const Edge& a, const Edge& b) {
    int a_idx_a = engine::grid::grid_position_to_index(a.gp_a, map.xmax);
    int a_idx_b = engine::grid::grid_position_to_index(a.gp_b, map.xmax);
    int b_idx_a = engine::grid::grid_position_to_index(b.gp_a, map.xmax);
    int b_idx_b = engine::grid::grid_position_to_index(b.gp_b, map.xmax);

    // either:
    // sort by the first gridpoint in the edge,
    // or if they're equal sort by the second grid point
    return (a_idx_a < b_idx_a) || (a_idx_a == b_idx_a && a_idx_b < b_idx_b);
  };
  std::sort(edges.begin(), edges.end(), p);

  const auto pred = [](const Edge& a, const Edge& b) { return a == b; };
  edges.erase(std::unique(edges.begin(), edges.end(), pred), edges.end());

  // Once they're sorted and unqiue... create entt representation
  for (const auto& edge : edges) {
    auto e = create_transform(r);
    r.emplace<Edge>(e, edge);
  }
};

void
instantiate_edges(entt::registry& r, MapComponent& map)
{
  for (const auto& [e, edge_c] : r.view<Edge>().each()) {
    const glm::vec2 ga = engine::grid::grid_space_to_world_space_center(edge_c.gp_a, map.tilesize);
    const glm::vec2 gb = engine::grid::grid_space_to_world_space_center(edge_c.gp_b, map.tilesize);

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

    DataSolidWall desc;
    desc.pos = center;
    desc.size = new_size;

    if (auto* door_c = r.try_get<DoorComponent>(e))
      desc.colour = { 1.0f, 0.0f, 0.0f, 1.0f };

    add_components(r, e, desc);
    set_colour(r, e, desc.colour);
  }
};

} // namespace game2d
