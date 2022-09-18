#include "dungeon.hpp"

#include "engine/maths/maths.hpp"
#include "game/components/components.hpp"
#include "game/helpers/fov.hpp"
#include "game/helpers/line.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "resources/colour.hpp"

#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace game2d {

const int GRID_SIZE = 16;

std::vector<entt::entity>
grid_entities_at(entt::registry& r, int x, int y)
{
  std::vector<entt::entity> results;
  const auto& view_grid_entities = r.view<const GridTileComponent>();
  view_grid_entities.each([&results, &x, &y](auto entity, const auto& grid) {
    if (x == grid.x && y == grid.y)
      results.push_back(entity);
  });
  return results;
}

void
create_room(entt::registry& r, const Room& room)
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
      SpriteColourComponent scc = create_colour(r, e, et);

      glm::ivec2 grid_index = { room.x + x, room.y + y };
      glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);
      t.position = { world_position.x, world_position.y, 0 };

      r.emplace<SpriteComponent>(e, s);
      r.emplace<TransformComponent>(e, t);
      r.emplace<SpriteColourComponent>(e, scc);

      bool contained_floor = false;

      std::vector<entt::entity> entities = grid_entities_at(r, grid_index.x, grid_index.y);
      for (const auto& entity : entities) {
        EntityTypeComponent type = r.get<EntityTypeComponent>(entity);
        if (type.type == EntityType::floor) {
          // leave it
          contained_floor = true;
        } else {
          r.destroy(entity);
        }
      }

      if (!contained_floor)
        r.emplace<GridTileComponent>(e, grid_index.x, grid_index.y);
      else
        r.destroy(e);

      // if (x != 0 && y != 0 && x != room.w - 1 && y != room.h - 1)
      //   r.emplace<HealthComponent>(e, 1); // give inner walls health
    }
  }
};

void
create_tunnel_floor(entt::registry& r, const Dungeon& d, std::vector<std::pair<int, int>>& coords)
{
  for (const auto& coord : coords) {
    int x = coord.first;
    int y = coord.second;

    EntityType et = EntityType::floor;
    entt::entity e = create_gameplay(r, et);
    SpriteComponent s = create_sprite(r, e, et);
    TransformComponent t = create_transform(r, e);
    SpriteColourComponent scc = create_colour(r, e, et);

    glm::ivec2 pos = engine::grid::grid_space_to_world_space({ x, y }, GRID_SIZE);
    t.position = { pos.x, pos.y, 0 };

    r.emplace<SpriteComponent>(e, s);
    r.emplace<TransformComponent>(e, t);
    r.emplace<SpriteColourComponent>(e, scc);

    std::vector<entt::entity> entities = grid_entities_at(r, x, y);
    for (const auto& entity : entities)
      r.destroy(entity);
    r.emplace<GridTileComponent>(e, x, y);
  }
}

// Create an L-shaped tunnel between two points
void
create_tunnel(entt::registry& r, const Dungeon& d, int x1, int y1, int x2, int y2)
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

//
// entry functions
//

void
generate_dungeon(entt::registry& r, const Dungeon& d, int step)
{
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();

  // destroy any grid tiles
  const auto& view_grid_entities = r.view<const GridTileComponent>();
  view_grid_entities.each([&r](auto entity, const auto& grid) { r.destroy(entity); });

  int offset_x = 15;
  int offset_y = 5;

  // create all the tiles
  for (int x = 0; x < d.width; x++) {
    for (int y = 0; y < d.height; y++) {
      EntityType et = EntityType::wall;

      entt::entity e = create_gameplay(r, et);
      SpriteComponent s = create_sprite(r, e, et);
      TransformComponent t = create_transform(r, e);
      SpriteColourComponent scc = create_colour(r, e, et);

      glm::ivec2 grid_index = { offset_x + x, offset_y + y };
      glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);
      t.position = { world_position.x, world_position.y, 0 };

      r.emplace<SpriteComponent>(e, s);
      r.emplace<TransformComponent>(e, t);
      r.emplace<GridTileComponent>(e, grid_index.x, grid_index.y);

      if (x != 0 && y != 0 && x != d.width - 1 && y != d.height - 1)
        r.emplace<HealthComponent>(e, 1); // give inner walls health

      r.emplace<SpriteColourComponent>(e, scc);
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

    int room_width = static_cast<int>(engine::rand_det_s(rnd.rng, room_min_size, room_max_size));
    int room_height = static_cast<int>(engine::rand_det_s(rnd.rng, room_min_size, room_max_size));
    int x = static_cast<int>(engine::rand_det_s(rnd.rng, 0, d.width - room_width - 1));
    int y = static_cast<int>(engine::rand_det_s(rnd.rng, 0, d.height - room_height - 1));

    Room room{ offset_x + x, offset_y + y, room_width, room_height };

    // Check if the room overlaps with any of the rooms
    const auto it =
      std::find_if(rooms.begin(), rooms.end(), [&room](const Room& other) { return rooms_overlap(room, other); });
    if (it != rooms.end())
      continue; // overlap; skip this room

    create_room(r, room);

    // dig out a tunnel between this room and the previous one
    bool starting_room = max_room_idx == 0;
    if (!starting_room) {
      auto r0_center = room_center(rooms[rooms.size() - 1]);
      auto r1_center = room_center(room);
      create_tunnel(r, d, r0_center.x, r0_center.y, r1_center.x, r1_center.y);
    }

    rooms.push_back(room);
  }

  //
  // Gameplay logic!
  //

  // Put a player in a room
  // limitation: currently all player put in same spot
  const auto& view = r.view<TransformComponent, const PlayerComponent>();
  view.each([&rooms](auto entity, TransformComponent& t, const PlayerComponent& p) {
    if (rooms.size() > 0) {
      auto room = rooms[0];
      auto center = room_center(room);
      glm::ivec2 pos = engine::grid::grid_space_to_world_space(center, GRID_SIZE);
      t.position = { pos.x, pos.y, 0 };
    }
  });

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

void
update_dungeon_system(entt::registry& r)
{
  const auto& player_view = r.view<PlayerComponent>();
  const auto player_entity = player_view.front();
  const auto& player_transform = r.get<TransformComponent>(player_entity);
  glm::ivec2 player_grid_pos =
    engine::grid::world_space_to_grid_space({ player_transform.position.x, player_transform.position.y }, 16);

  const auto& view_grid_tiles = r.view<SpriteColourComponent, const GridTileComponent, const EntityTypeComponent>();

  view_grid_tiles.each([&r, &player_grid_pos](auto e, auto& scc, const auto& grid, const auto& type) {
    const int distance_x = glm::abs(grid.x - player_grid_pos.x);
    const int distance_y = glm::abs(grid.y - player_grid_pos.y);
    const int dst = 4;
    const bool within_distance = distance_x < dst && distance_y < dst;

    // If it's within the distance, make it visible
    if (within_distance) {
      r.emplace_or_replace<VisibleComponent>(e);
      if (r.try_get<NotVisibleComponent>(e))
        r.remove<NotVisibleComponent>(e);
      if (r.try_get<NotVisibleButPreviouslySeenComponent>(e))
        r.remove<NotVisibleButPreviouslySeenComponent>(e);
    }
    // If it's not within the distance, hide it
    else {

      // if it's already got NotVisibleButPreviouslySeenComponent,
      auto seen_before = r.try_get<NotVisibleButPreviouslySeenComponent>(e);
      if (seen_before)
        return; // change nothing

      // if it was Visible, set as NotVisibleButPreviouslySeenComponent
      if (auto visible = r.try_get<VisibleComponent>(e)) {
        r.remove<VisibleComponent>(e);
        r.emplace_or_replace<NotVisibleButPreviouslySeenComponent>(e);
      }
      // if it was not Visible, just set it back to not visible
      else {
        r.emplace_or_replace<NotVisibleComponent>(e);
      }
    }
  });
}

} // namespace game2d