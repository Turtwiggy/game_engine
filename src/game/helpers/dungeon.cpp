#include "dungeon.hpp"

#include "engine/maths/maths.hpp"
#include "game/components/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"

#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace game2d {

const int GRID_SIZE = 16;

void
create_room(entt::registry& r, Dungeon& d, const Room& room)
{
  int x1 = room.x;
  int y1 = room.y;
  int x2 = x1 + room.w;
  int y2 = y1 + room.h;

  for (int x = 0; x < room.w; x++) {
    for (int y = 0; y < room.h; y++) {

      EntityType et = EntityType::floor;

      if (x == 0)
        et = EntityType::wall;
      if (y == 0)
        et = EntityType::wall;
      if (x == room.w - 1)
        et = EntityType::wall;
      if (y == room.h - 1)
        et = EntityType::wall;

      entt::entity e = create_gameplay(r, et);
      SpriteComponent s = create_sprite(r, e, et);
      TransformComponent t = create_transform(r, e);

      glm::ivec2 grid_index = { room.x + x, room.y + y };
      glm::ivec2 grid_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);
      t.position = { grid_position.x, grid_position.y, 0 };

      r.emplace<SpriteComponent>(e, s);
      r.emplace<TransformComponent>(e, t);

      // add entity as only entity in grid position
      std::pair<int, int> tilemap_index = { grid_index.x, grid_index.y };
      if (d.tilemap.contains(tilemap_index)) {
        std::vector<entt::entity>& entities = d.tilemap[tilemap_index];

        bool contained_floor = false;
        {
          for (int j = 0; const auto& entity : entities) {
            TagComponent tag = r.get<TagComponent>(entities[j]);
            if (tag.tag == "FLOOR") {
              // leave it
              contained_floor = true;
            } else {
              r.destroy(entity);
              entities[j] = entt::null;
            }
            ++j;
          }
        }

        // remove all entt::null from entities list
        entities.erase(
          std::remove_if(entities.begin(), entities.end(), [](const entt::entity& e) { return e == entt::null; }),
          entities.end());

        if (!contained_floor)
          entities.push_back(e);
        else
          r.destroy(e); // dont need this created entity
      } else {
        d.tilemap[tilemap_index] = { e };
      }

      //
    }
  }
};

// inspired by:
// https://github.com/scikit-image/scikit-image/blob/main/skimage/draw/_draw.pyx
void
create_line(int r0, int c0, int r1, int c1, std::vector<std::pair<int, int>>& results)
{
  int r = r0;
  int c = c0;
  int dr = glm::abs(r1 - r0);
  int dc = glm::abs(c1 - c0);
  int sc = c1 - c > 0 ? 1 : -1;
  int sr = r1 - r > 0 ? 1 : -1;

  int steep = 0;
  if (dr > dc) {
    steep = 1;
    std::swap(c, r);
    std::swap(dc, dr);
    std::swap(sc, sr);
  }
  int d = (2 * dr) - dc;

  std::pair<int, int> res;

  for (int i = 0; i < dc; i++) {
    if (steep) {
      res.first = c;
      res.second = r;
      results.push_back(res);
    } else {
      res.first = r;
      res.second = c;
      results.push_back(res);
    }
    while (d >= 0) {
      r = r + sr;
      d = d - (2 * dc);
    }
    c = c + sc;
    d = d + (2 * dr);
  }

  // res.first = r1;
  // res.second = c1;
  // results.push_back(res);
};

void
create_tunnel_floor(entt::registry& r, Dungeon& d, std::vector<std::pair<int, int>>& coords)
{
  for (const auto& coord : coords) {
    int x = coord.first;
    int y = coord.second;

    EntityType et = EntityType::floor;
    entt::entity e = create_gameplay(r, et);
    SpriteComponent s = create_sprite(r, e, et);
    TransformComponent t = create_transform(r, e);

    glm::ivec2 pos = engine::grid::grid_space_to_world_space({ x, y }, GRID_SIZE);
    t.position = { pos.x, pos.y, 0 };

    r.emplace<SpriteComponent>(e, s);
    r.emplace<TransformComponent>(e, t);

    // add entity as only entity in grid position
    std::pair<int, int> tilemap_index = { x, y };
    if (d.tilemap.contains(tilemap_index)) {
      std::vector<entt::entity>& entities = d.tilemap[tilemap_index];
      for (const auto& entity : entities)
        r.destroy(entity);
      entities.clear();
      entities.push_back(e);
    } else {
      d.tilemap[tilemap_index] = { e };
    }
  }
}

// Create an L-shaped tunnel between two points
void
create_tunnel(entt::registry& r, Dungeon& d, int x1, int y1, int x2, int y2)
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
  create_tunnel_floor(r, d, line_0);

  // b) corner_x, corner_y to x2, y2
  std::vector<std::pair<int, int>> line_1;
  create_line(corner_x, corner_y, x2, y2, line_1);
  create_tunnel_floor(r, d, line_1);
};

bool
rooms_overlap(const Room& r0, const Room& r1)
{
  PhysicsObject p0;
  p0.x_tl = r0.x;
  p0.y_tl = r0.y;
  p0.w = r0.w;
  p0.h = r0.h;

  PhysicsObject p1;
  p1.x_tl = r1.x;
  p1.y_tl = r1.y;
  p1.w = r1.w;
  p1.h = r1.h;

  return collides(p0, { p1 });
}

constexpr glm::ivec2
room_center(const Room& r)
{
  return { (r.x + r.x + r.w) / 2, (r.y + r.y + r.h) / 2 };
};

void
generate_dungeon(entt::registry& r, Dungeon& d, int step)
{
  // delete all entities, clear dungeon tilemap
  for (auto const& [key, val] : d.tilemap)
    for (const auto& entity : val)
      r.destroy(entity);
  d.tilemap.clear();

  for (int x = 0; x < d.width; x++) {
    for (int y = 0; y < d.height; y++) {
      EntityType et = EntityType::wall;
      entt::entity e = create_gameplay(r, et);
      SpriteComponent s = create_sprite(r, e, et);
      TransformComponent t = create_transform(r, e);
      glm::ivec2 grid_index = { x, y };
      glm::ivec2 grid_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);
      t.position = { grid_position.x, grid_position.y, 0 };

      r.emplace<SpriteComponent>(e, s);
      r.emplace<TransformComponent>(e, t);

      // add entity as only entity in grid position
      std::pair<int, int> tilemap_index = { grid_index.x, grid_index.y };
      if (d.tilemap.contains(tilemap_index)) {
        std::vector<entt::entity>& entities = d.tilemap[tilemap_index];
        for (const auto& entity : entities)
          r.destroy(entity);
        entities.clear();
        entities.push_back(e);
      } else {
        d.tilemap[tilemap_index] = { e };
      }
    }
  }

  engine::RandomState rnd;
  rnd.rng.seed(0);
  const int room_min_size = 6;
  const int room_max_size = 10;
  const int max_rooms = 30;

  std::vector<Room> rooms;

  std::cout << "step is: " << step << "max is: " << max_rooms << "\n";
  if (step == -1)
    step = max_rooms + 1;

  for (int max_room_idx = 0; max_room_idx < max_rooms; max_room_idx++) {
    if (max_room_idx > step)
      break;

    int room_width = engine::rand_det_s(rnd.rng, room_min_size, room_max_size);
    int room_height = engine::rand_det_s(rnd.rng, room_min_size, room_max_size);
    int x = engine::rand_det_s(rnd.rng, 0, d.width - room_width - 1);
    int y = engine::rand_det_s(rnd.rng, 0, d.height - room_height - 1);

    Room room;
    room.x = x;
    room.y = y;
    room.w = room_width;
    room.h = room_height;

    // Check if the room overlaps with any of the rooms
    const auto it =
      std::find_if(rooms.begin(), rooms.end(), [&room](const Room& other) { return rooms_overlap(room, other); });
    if (it != rooms.end())
      continue; // overlap; skip this room

    create_room(r, d, room);

    // this is the first valid room
    bool starting_room = max_room_idx == 0;

    if (starting_room) {
      // Put the player in it
      // limitation: currently all player put in same spot
      const auto& view = r.view<TransformComponent, const PlayerComponent>();
      view.each([&x, &y, &room](auto entity, TransformComponent& t, const PlayerComponent& p) {
        auto center = room_center(room);
        glm::ivec2 pos = engine::grid::grid_space_to_world_space(center, GRID_SIZE);
        t.position = { pos.x, pos.y, 0 };
      });
    }

    // dig out a tunnel between this room and the previous one
    if (!starting_room) {
      auto r0_center = room_center(rooms[rooms.size() - 1]);
      auto r1_center = room_center(room);
      create_tunnel(r, d, r0_center.x, r0_center.y, r1_center.x, r1_center.y);
    }

    rooms.push_back(room);
  }

  // Put a shopkeeper in a room
  const auto& shopkeeper_view = r.view<ShopKeeperComponent, TransformComponent>();
  shopkeeper_view.each([&rooms](ShopKeeperComponent& sk, TransformComponent& t) {
    if (rooms.size() > 1) {
      auto room = rooms[1];
      auto center = room_center(room);
      glm::ivec2 pos = engine::grid::grid_space_to_world_space(center, GRID_SIZE);
      t.position = { pos.x, pos.y, 0 };
    }
  });
};

} // namespace game2d