#include "helpers.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "helpers/line.hpp"
#include "maths/grid.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/grid/components.hpp"
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

      // else, create a floor? do nothing?
      // nada

      // world gridspace
      // update map representation
      const auto grid_pos = glm::ivec2{ room.tl.x + x, room.tl.y + y };
      const int global_idx = engine::grid::grid_position_to_index(grid_pos, xmax);
      wall_or_floors[global_idx] = create_as_wall ? 1 : 0;
    }
  }
};

std::vector<Room>
create_all_rooms(DungeonGenerationCriteria& data,
                 const MapComponent& map,
                 std::vector<int>& wall_or_floors,
                 engine::RandomState& rnd)
{
  const int xmax = map.xmax;
  const int max_rooms = data.max_rooms;
  const int room_size_min = data.room_size_min;
  const int room_size_max = data.room_size_max;

  // Create some rooms
  const int map_width = map.xmax;
  const int map_height = map.ymax;

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

    create_room(room, wall_or_floors, xmax);
    rooms.push_back(room);

    // dig out some TUNNELS between this room and the previous one
    if (i == 0) // starting room cant build a tunnel :()
      continue;
    {
      const Room& prev_room = rooms[rooms.size() - 2];
      const int horiz_or_vert = int(engine::rand_det_s(rnd.rng, 0, 1));
      const auto x1 = room.aabb.center.x;
      const auto y1 = room.aabb.center.y;
      const auto x2 = prev_room.aabb.center.x;
      const auto y2 = prev_room.aabb.center.y;
      int corner_x = 0;
      int corner_y = 0;
      if (horiz_or_vert == 1) {
        // move horisontally, then vertically
        corner_x = x2;
        corner_y = y1;
      } else {
        // move vertically, then horizontally
        corner_x = x1;
        corner_y = y2;
      }

      // generate coordinates based on bresenham line agoorithm
      // SECRET TUNNELLLLL.... SECRET TUNNELLLLL....

      // a) x1, y1 to corner_x, corner_y
      const auto line_0 = create_line(room.aabb.center.x, room.aabb.center.y, corner_x, corner_y);
      for (const auto& [x, y] : line_0) {
        const int idx = engine::grid::grid_position_to_index({ x, y }, xmax);
        wall_or_floors[idx] = 0; // set as floor
      }

      // b) corner_x, corner_y to x2, y2
      const auto line_1 = create_line(corner_x, corner_y, prev_room.aabb.center.x, prev_room.aabb.center.y);
      for (const auto& [x, y] : line_1) {
        const int idx = engine::grid::grid_position_to_index({ x, y }, xmax);
        wall_or_floors[idx] = 0; // set as floor
      }
    }

  } // end iterating rooms

  return rooms;
};

// void
// instantiate_map(entt::registry& r, const MapComponent& map, const std::vector<int>& map_gen)
// {
//   const auto& xmax = map.xmax;
//   const auto& ymax = map.ymax;
//   const auto& tilesize = map.tilesize;
//   // Create state from the map
//   for (int i = 0; i < xmax * ymax; i++) {
//     const auto worldspace = engine::grid::index_to_world_position(i, xmax, ymax, tilesize);
//     const bool is_wall = map_gen[i] == 1;
//     if (is_wall) {
//       const auto wall_e = create_gameplay(r, EntityType::solid_wall);
//       set_position(r, wall_e, worldspace);
//       set_size(r, wall_e, { tilesize, tilesize });
//       r.emplace_or_replace<PathfindComponent>(wall_e, -1);
//       // add to pathfinding map?
//       std::vector<entt::entity>& ents = map.map[i];
//       ents.push_back(wall_e);
//     }
//   }
// };

void
generate_dungeon(entt::registry& r)
{
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  // the pathfinding map to base dungeon generation on (sizes)
  const auto& map = get_first_component<MapComponent>(r);

  const int seed = 0;
  static engine::RandomState rnd(seed);

  // represents state of the entire grid. set to 1 as everything is a wall
  std::vector<int> map_gen(map.xmax * map.ymax, 1);

  DungeonGenerationCriteria dungeon_parameters;
  const auto rooms = create_all_rooms(dungeon_parameters, map, map_gen, rnd);
  // pass to the spaceship system?

  // instantiate_map(r, map, map_gen);

  // set_generated_entity_positions(editor, game, d, rnd);
  // set_player_positions(editor, game, d, rnd);

  // Where's the spawn?
  // const auto& first_room = rooms[0];
  // bug: placing player on generated entities.

  // Where's the exit?
  // bug: exit placed on generated entities. not so bad?
  // Room& room = d.rooms[d.rooms.size() - 1];
  // glm::ivec2 middle = room_center(room);
  // create_dungeon_entity(editor, game, EntityType::tile_type_exit, middle);

}; // end generate dungeon

} // namespace game2d

//
//
//
//
//

// EntityType
// generate_item(const int floor, engine::RandomState& rnd)
// {
//   const std::vector<std::pair<int, std::pair<EntityType, int>>> difficulty_map{
//     // clang-format off
//     { 1, { EntityType::potion, 80 } },
//     { 2, { EntityType::sword, 15 } },
//     { 3, { EntityType::shield, 15 } },
//     { 4, { EntityType::scroll_damage_nearest, 5 } },
//     // { 1, { EntityType::scroll_damage_selected_on_grid, 5 } },
//     // { 2, { EntityType::crossbow, 15 } },
//     // { 2, { EntityType::bolt, 15 } },
//     // clang-format on
//   };

//   std::map<EntityType, int> weights;
//   for (const auto& [dict_floor, dict_type] : difficulty_map) {

//     // only interested in weights at our floor
//     if (dict_floor > floor)
//       break;

//     // sum the weights!
//     const auto& entity_type = dict_type.first;
//     const auto& entity_weight = dict_type.second;
//     if (weights.contains(entity_type))
//       weights[entity_type] += entity_weight;
//     else
//       weights[entity_type] = entity_weight;
//   }

//   int total_weight = 0;
//   for (const auto& [k, w] : weights)
//     total_weight += w;

//   const float random = engine::rand_det_s(rnd.rng, 0, total_weight);
//   int weight_acculum = 0;
//   for (const auto& [k, w] : weights) {
//     weight_acculum += w;
//     if (weight_acculum >= random)
//       return k;
//   }
//   return EntityType::potion;
// };

// EntityType
// generate_monster(const int floor, engine::RandomState& rnd)
// {
//   const std::vector<std::pair<int, std::pair<EntityType, int>>> difficulty_map{
//     // clang-format off
//     { 1, { EntityType::actor_bat, 80 } },
//     { 2, { EntityType::actor_troll, 15 } },
//     { 3, { EntityType::actor_troll, 30 } },
//     { 4, { EntityType::actor_troll, 60 } }
//     // clang-format on
//   };

//   std::map<EntityType, int> weights;
//   for (const auto& [dict_floor, dict_type] : difficulty_map) {

//     // only interested in weights at our floor
//     if (dict_floor > floor)
//       break;

//     // sum the weights!
//     const auto& entity_type = dict_type.first;
//     const auto& entity_weight = dict_type.second;
//     if (weights.contains(entity_type))
//       weights[entity_type] += entity_weight;
//     else
//       weights[entity_type] = entity_weight;
//   }

//   int total_weight = 0;
//   for (const auto& [k, w] : weights)
//     total_weight += w;

//   const float random = engine::rand_det_s(rnd.rng, 0, total_weight);
//   int weight_acculum = 0;
//   for (const auto& [k, w] : weights) {
//     weight_acculum += w;
//     if (weight_acculum >= random)
//       return k;
//   }
//   return EntityType::actor_bat;
// };