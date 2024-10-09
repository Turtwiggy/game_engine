#include "rooms_random.hpp"

#include "components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/priority_queue.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/line.hpp"
#include "engine/maths/maths.hpp"
#include "modules/map/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

namespace game2d {

std::vector<entt::entity>
inside_room(entt::registry& r, const glm::ivec2& gridpos)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  if (gp_out_of_bounds(gridpos, map_c.xmax, map_c.ymax))
    return {};
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

bool
collide(const RoomAABB& one, const RoomAABB& two)
{
  const glm::ivec2 one_tl_pos = { one.center.x - one.size.x / 2, one.center.y - one.size.y / 2 };
  const glm::ivec2 two_tl_pos = { two.center.x - two.size.x / 2, two.center.y - two.size.y / 2 };

  // collision x-axis?
  bool collision_x = one_tl_pos.x + one.size.x > two_tl_pos.x && two_tl_pos.x + two.size.x > one_tl_pos.x;

  // collision y-axis?
  bool collision_y = one_tl_pos.y + one.size.y > two_tl_pos.y && two_tl_pos.y + two.size.y > one_tl_pos.y;

  return collision_x && collision_y;
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

DungeonIntermediate
generate_rooms(entt::registry& r, const DungeonGenerationCriteria& data, engine::RandomState& rnd)
{
  const auto& map = get_first_component<MapComponent>(r);

  std::vector<FloorType> floor_types(map.xmax * map.ymax, FloorType::WALL);

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
    auto create_room = [](const Room& room, std::vector<FloorType>& wall_or_floors, const int xmax) {
      const auto& w = room.aabb.value().size.x;
      const auto& h = room.aabb.value().size.y;

      for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
          // by commenting out below,
          // set the entire generated room to "floor"
          const auto grid_pos = glm::ivec2{ room.tl.value().x + x, room.tl.value().y + y };
          const auto global_idx = engine::grid::grid_position_to_index(grid_pos, xmax);
          wall_or_floors[global_idx] = FloorType::FLOOR;
        }
      }
    };
    create_room(room, floor_types, xmax);
    rooms.push_back(room);

  } // end iterating rooms

  DungeonIntermediate results;
  results.rooms = rooms;
  results.floor_types = floor_types;

  // give to entt
  for (const Room& room : results.rooms)
    create_empty<Room>(r, room);

  return results;
};

// i.e. take all the isolated islands of floor tiles and convert them in to the room representation
[[nodiscard]] std::vector<int>
convert_floor_islands_to_room(entt::registry& r,
                              const MapComponent& map_c,
                              const int from_idx,
                              const DungeonIntermediate& dresults)
{
  std::map<int, int> pos_to_distance;
  pos_to_distance[from_idx] = 0;

  PriorityQueue<int> frontier;
  frontier.enqueue(from_idx, 0);

  std::vector<int> results;

  while (frontier.size() > 0) {
    const int current_idx = frontier.dequeue();
    const auto current = engine::grid::index_to_grid_position(current_idx, map_c.xmax, map_c.ymax);

    if (!(dresults.floor_types[current_idx] == FloorType::FLOOR))
      continue; // starting tile isnt floor
    if (inside_room(r, { current.x, current.y }).size() > 0)
      continue; // gone to a different room
    results.push_back(current_idx);

    // check neighbours
    const auto neighbours_idxs = engine::grid::get_neighbour_indicies(current.x, current.y, map_c.xmax, map_c.ymax);

    for (const auto& [dir, nidx] : neighbours_idxs) {
      const auto neighbour_pos = engine::grid::index_to_grid_position(nidx, map_c.xmax, map_c.ymax);

      if (!(dresults.floor_types[nidx] == FloorType::FLOOR))
        continue; // starting tile isnt floor
      if (inside_room(r, { current.x, current.y }).size() > 0)
        continue; // gone to a different room

      int distance = pos_to_distance[current_idx] + 1;

      // if a distance value already existed, take the smaller distance
      if (pos_to_distance.contains(nidx))
        pos_to_distance[nidx] = glm::min(distance, pos_to_distance[nidx]);

      // no distance value: insert a new one
      else {
        pos_to_distance[nidx] = distance;
        frontier.enqueue(nidx, 0);
      }
    }
  }

  return results;
};

void
convert_tunnels_to_rooms(entt::registry& r, const DungeonIntermediate& results)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  for (int xy = 0; xy < map_c.xmax * map_c.ymax; xy++) {

    std::vector<int> room_idxs = convert_floor_islands_to_room(r, map_c, xy, results);
    if (room_idxs.size() == 0)
      continue;
    fmt::println("connected room found of size: {}", room_idxs.size());

    Room room;
    room.tiles_idx = room_idxs;
    auto room_e = create_empty<Room>(r, room);

    RoomName room_name_c;
    room_name_c.name = "Tunnel";
    r.emplace<RoomName>(room_e, room_name_c);
  }
};

void
generate_tunnel(entt::registry& r, const Room a, const Room b, DungeonIntermediate& result)
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
    auto idx = engine::grid::grid_position_to_index({ x, y }, map_c.xmax);
    result.floor_types[idx] = FloorType::FLOOR;
  }

  // b) corner_x, corner_y to x2, y2
  for (const auto& [x, y] : line_1) {
    auto idx = engine::grid::grid_position_to_index({ x, y }, map_c.xmax);
    result.floor_types[idx] = FloorType::FLOOR;
  }
  //
};

// create in-between rooms (i.e. tunnels or corridors) between rooms
// 1) set the tiles in the results to "floor"
// 2) create the sections outside the room as a new "room" and add to entt
void
connect_rooms_via_nearest_neighbour(entt::registry& r, DungeonIntermediate& result)
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
};

std::vector<int>
get_empty_slots_idxs(entt::registry& r, const MapComponent& map_c, const Room& room)
{
  std::vector<int> results;

  for (const auto idx : room.tiles_idx) {

    bool free = true;

    for (const auto& e : map_c.map[idx]) {

      // not free if someone is there...
      if (auto* body_c = r.try_get<DefaultBody>(e))
        free = false;

      // not free if anything is there...
      free = false;

      //
    }

    if (free)
      results.push_back(idx);
  }

  return results;
};

} // namespace game2d