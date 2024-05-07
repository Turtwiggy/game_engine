#include "entt/helpers.hpp"

#include "actors.hpp"
#include "components.hpp"

#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/ai_pathfinding/components.hpp"
#include "modules/selected_interactions/components.hpp"
#include "physics/helpers.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace game2d {

//
// create a blank map.
// https://bfnightly.bracketproductions.com/rustbook/chapter_23.html
//
//
// remember to update map representation to update pathfinding.
// assume state has been taken care of. (deleting old dungeon)
//

void
create_room(const Room& room, std::vector<int>& wall_or_floors, const int xmax)
{
  const auto& room_width = room.aabb.size.x;
  const auto& room_height = room.aabb.size.y;
  for (int xy = 0; xy < room_width * room_height; xy++) {
    const auto room_xy = engine::grid::index_to_grid_position(xy, room_width, room_height);

    // create walls on the outside of the room
    bool create_as_wall = false;
    if (room_xy.x == 0 || room_xy.x == room.aabb.size.x - 1)
      create_as_wall = true;
    if (room_xy.y == 0 || room_xy.y == room.aabb.size.y - 1)
      create_as_wall = true;

    // else, create a floor? do nothing?
    // nada

    // update map representation
    const auto global_gridpos = glm::ivec2{ room.tl.x + room_xy.x, room.tl.y + room_xy.y };
    const int global_idx = engine::grid::grid_position_to_index(global_gridpos, xmax);
    wall_or_floors[global_idx] = create_as_wall ? 1 : 0;
  }
};

void
generate_dungeon(entt::registry& r)
{
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  // the pathfinding map to base dungeon generation on (sizes)
  auto& map = get_first_component<MapComponent>(r);
  const auto& xmax = map.xmax;
  const auto& ymax = map.ymax;
  const auto& tilesize = map.tilesize;

  const int seed = 0;
  engine::RandomState rnd;
  rnd.rng.seed(seed);

  // Room paramters
  constexpr int room_min_size = 6; // grid-squares
  constexpr int room_max_size = 10;
  constexpr int max_rooms = 1;

  // Create some rooms
  const int width = xmax;
  const int height = ymax;

  // represents state of the entire grid
  std::vector<int> floor_or_wall_global_map; // 1 = wall
  floor_or_wall_global_map.resize(width * height);
  for (int i = 0; i < width * height; i++)
    floor_or_wall_global_map[i] = 1; // set all as walls

  std::vector<Room> rooms;
  for (int i = 0; i < max_rooms; i++) {
    //
    const int gen_room_width = glm::roundEven(float(engine::rand_det_s(rnd.rng, room_min_size, room_max_size)));
    const int gen_room_height = glm::roundEven(float(engine::rand_det_s(rnd.rng, room_min_size, room_max_size)));
    const int l = int(engine::rand_det_s(rnd.rng, 0, width - gen_room_width - 1));
    const int t = int(engine::rand_det_s(rnd.rng, 0, height - gen_room_height - 1));
    const auto tl = glm::ivec2{ t, l };

    Room room;
    room.tl = tl;
    room.aabb.center = { tl.x + (gen_room_width / 2), tl.y + (gen_room_height / 2) };
    room.aabb.size = { gen_room_width, gen_room_height };

    // if new room collides with existin groom, skip this room.
    const auto room_collides = [&room](const Room& other) { return collide(room.aabb, other.aabb); };
    const auto it = std::find_if(rooms.begin(), rooms.end(), room_collides);
    if (it != rooms.end())
      continue;
    std::cout << "generating room of size: " << gen_room_width << " ," << gen_room_height << std::endl;

    create_room(room, floor_or_wall_global_map, xmax);
    rooms.push_back(room);

    // dig out some TUNNELS between this room and the previous one
    if (i == 0) // starting room cant build a tunnel :()
      continue;
    // SECRET TUNNELLLLL.... SECRET TUNNELLLLL....
    // const auto create_tunnel = [](const Room& a, const Room& b) {
    //   //
    // };
    // create_tunnel(rooms[rooms.size() - 1], room);

  } // end iterating rooms

  // Create state from the map
  //
  for (int i = 0; i < xmax * ymax; i++) {
    const auto worldspace = engine::grid::index_to_world_position(i, xmax, ymax, tilesize);

    const bool is_wall = floor_or_wall_global_map[i] == 1;
    if (is_wall) {
      const auto wall_e = create_gameplay(r, EntityType::solid_wall);
      set_position(r, wall_e, worldspace);
      set_size(r, wall_e, { tilesize / 2, tilesize / 2 });
      r.emplace_or_replace<PathfindComponent>(wall_e, -1);

      // add to pathfinding map?
      std::vector<entt::entity>& ents = map.map[i];
      ents.push_back(wall_e);
    }
  }

} // end generate dungeon

} // namespace game2d