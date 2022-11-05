#include "create.hpp"

#include "engine/maths/grid.hpp"
#include "game/helpers/line.hpp"
#include "game/modules/dungeon/helpers/rooms.hpp"
#include "game/modules/fov/components.hpp"
namespace game2d {

const int GRID_SIZE = 16;

entt::entity
create_dungeon_entity(GameEditor& editor, Game& game, EntityType et, const glm::ivec2& grid_index)
{
  auto& r = game.state;
  glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);

  entt::entity e = create_gameplay(editor, game, et);
  SpriteComponent s = create_sprite(editor, r, e, et);
  TransformComponent t = create_transform(r, e);
  SpriteColourComponent scc = create_colour(editor, r, e, et);

  t.position = { world_position.x, world_position.y, 0 };

  r.emplace<SpriteComponent>(e, s);
  r.emplace<SpriteColourComponent>(e, scc);
  r.emplace<TransformComponent>(e, t);
  r.emplace<FovComponent>(e);

  return e;
};

void
create_all_walls(GameEditor& editor, Game& game, Dungeon& d)
{
  for (int x = 0; x < d.width; x++) {
    for (int y = 0; y < d.height; y++) {
      const glm::ivec2 grid_index = { x, y };
      const int index = d.width * grid_index.y + grid_index.x;
      entt::entity e = create_dungeon_entity(editor, game, EntityType::tile_type_wall, grid_index);
      StaticDungeonEntity se;
      se.entity = e;
      se.x = grid_index.x;
      se.y = grid_index.y;
      d.walls_and_floors[index] = se;
    }
  }
};

void
create_all_rooms(GameEditor& editor, Game& game, Dungeon& d, engine::RandomState rnd)
{
  constexpr int room_min_size = 6;
  constexpr int room_max_size = 10;
  constexpr int max_rooms = 30;
  int room_index = 0;

  for (int max_room_idx = 0; max_room_idx < max_rooms; max_room_idx++) {
    const int room_width = static_cast<int>(engine::rand_det_s(rnd.rng, room_min_size, room_max_size));
    const int room_height = static_cast<int>(engine::rand_det_s(rnd.rng, room_min_size, room_max_size));
    const int x = static_cast<int>(engine::rand_det_s(rnd.rng, 0, d.width - room_width - 1));
    const int y = static_cast<int>(engine::rand_det_s(rnd.rng, 0, d.height - room_height - 1));

    Room room;
    room.x1 = x;
    room.y1 = y;
    room.x2 = room.x1 + room_width;
    room.y2 = room.y1 + room_height;
    room.w = room_width;
    room.h = room_height;

    // Check if the room overlaps with any of the rooms
    const auto it =
      std::find_if(d.rooms.begin(), d.rooms.end(), [&room](const Room& other) { return rooms_overlap(room, other); });
    if (it != d.rooms.end())
      continue; // overlap; skip this room

    create_room(editor, game, d, room, ++room_index);

    // dig out a tunnel between this room and the previous one
    bool starting_room = max_room_idx == 0;
    if (!starting_room) {
      auto r0_center = room_center(d.rooms[d.rooms.size() - 1]);
      auto r1_center = room_center(room);
      create_tunnel(editor, game, d, r0_center.x, r0_center.y, r1_center.x, r1_center.y);
    }

    d.rooms.push_back(room);
  }
}

void
create_room(GameEditor& editor, Game& game, Dungeon& d, const Room& room, int room_index)
{
  auto& r = game.state;

  for (int x = 0; x < room.w; x++) {
    for (int y = 0; y < room.h; y++) {

      EntityType et = EntityType::tile_type_floor;
      if (x == 0 || x == room.w - 1)
        et = EntityType::tile_type_wall;
      if (y == 0 || y == room.h - 1)
        et = EntityType::tile_type_wall;

      const glm::ivec2 grid_index = { room.x1 + x, room.y1 + y };
      const int index = d.width * grid_index.y + grid_index.x;

      if (EntityType::tile_type_floor == et) {
        if (d.walls_and_floors[index].entity != entt::null)
          r.destroy(d.walls_and_floors[index].entity);
        entt::entity e = create_dungeon_entity(editor, game, et, grid_index);
        StaticDungeonEntity se;
        se.entity = e;
        se.x = grid_index.x;
        se.y = grid_index.y;
        se.room_index = room_index;
        d.walls_and_floors[index] = se;
      }
    }
  }
};

void
create_tunnel_floor(GameEditor& editor, Game& game, Dungeon& d, std::vector<std::pair<int, int>>& coords)
{
  auto& r = game.state;

  for (const auto& coord : coords) {
    const int index = d.width * coord.second + coord.first;

    if (d.walls_and_floors[index].entity != entt::null)
      r.destroy(d.walls_and_floors[index].entity);

    entt::entity entity =
      create_dungeon_entity(editor, game, EntityType::tile_type_floor, { coord.first, coord.second });

    StaticDungeonEntity se;
    se.entity = entity;
    se.x = coord.first;
    se.y = coord.second;
    d.walls_and_floors[index] = se;
  }
};

void
create_tunnel(GameEditor& editor, Game& game, Dungeon& d, int x1, int y1, int x2, int y2)
{
  int corner_x = 0;
  int corner_y = 0;

  // move horisontally, then vertically
  corner_x = x2;
  corner_y = y1;

  // move vertically, then horizontally
  // corner_x = x1;
  // corner_y = y2;

  // generate coordinates based on bresenham line agoorithm

  // a) x1, y1 to corner_x, corner_y
  std::vector<std::pair<int, int>> line_0;
  create_line(x1, y1, corner_x, corner_y, line_0);
  create_tunnel_floor(editor, game, d, line_0);

  // b) corner_x, corner_y to x2, y2
  std::vector<std::pair<int, int>> line_1;
  create_line(corner_x, corner_y, x2, y2, line_1);
  create_tunnel_floor(editor, game, d, line_1);
};

} // namespace game2d