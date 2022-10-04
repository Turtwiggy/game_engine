#include "dungeon.hpp"

#include "engine/maths/maths.hpp"
#include "game/components/components.hpp"
#include "game/helpers/fov.hpp"
#include "game/helpers/line.hpp"
#include "modules/camera/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "resources/colour.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <utility>
#include <vector>

namespace game2d {

const int GRID_SIZE = 16;

std::vector<entt::entity>
grid_entities_at(entt::registry& r, int x, int y)
{
  std::vector<entt::entity> results;
  const auto& view = r.view<const GridComponent>();
  view.each([&results, &x, &y](auto entity, const auto& grid) {
    if (x == grid.x && y == grid.y)
      results.push_back(entity);
  });
  return results;
}

void
get_neighbour_indicies(const int x,
                       const int y,
                       const int x_max,
                       const int y_max,
                       std::vector<std::pair<GridDirection, int>>& results)
{
  const int max_idx = x_max * y_max;
  const int idx_north = x_max * (y - 1) + x;
  const int idx_east = x_max * y + (x + 1);
  const int idx_south = x_max * (y + 1) + x;
  const int idx_west = x_max * y + (x - 1);
  // const int idx_north_east = x_max * (y - 1) + (x + 1);
  // const int idx_south_east = x_max * (y + 1) + (x + 1);
  // const int idx_south_west = x_max * (y + 1) + (x - 1);
  // const int idx_north_west = x_max * (y - 1) + (x - 1);
  const bool ignore_north = y <= 0;
  const bool ignore_east = x >= x_max - 1;
  const bool ignore_south = y >= y_max - 1;
  const bool ignore_west = x <= 0;
  // const bool ignore_north_east = ignore_north | ignore_east;
  // const bool ignore_south_east = ignore_south | ignore_east;
  // const bool ignore_south_west = ignore_south | ignore_west;
  // const bool ignore_north_west = ignore_north | ignore_west;

  if (!ignore_north && idx_north >= 0 && idx_north < max_idx)
    results.push_back({ GridDirection::north, idx_north });

  if (!ignore_east && idx_east >= 0 && idx_east < max_idx)
    results.push_back({ GridDirection::east, idx_east });

  if (!ignore_south && idx_south >= 0 && idx_south < max_idx)
    results.push_back({ GridDirection::south, idx_south });

  if (!ignore_west && idx_west >= 0 && idx_west < max_idx)
    results.push_back({ GridDirection::west, idx_west });

  // if (!ignore_north_east && idx_north_east >= 0 && idx_north_east < max_idx)
  //   results.push_back({ GridDirection::north_east, idx_north_east });

  // if (!ignore_south_east && idx_south_east >= 0 && idx_south_east < max_idx)
  //   results.push_back({ GridDirection::south_east, idx_south_east });

  // if (!ignore_south_west && idx_south_west >= 0 && idx_south_west < max_idx)
  //   results.push_back({ GridDirection::south_west, idx_south_west });

  // if (!ignore_north_west && idx_north_west >= 0 && idx_north_west < max_idx)
  //   results.push_back({ GridDirection::north_west, idx_north_west });
};

void
create_room(entt::registry& r, const Room& room)
{
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

      glm::ivec2 grid_index = { room.x1 + x, room.y1 + y };
      glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);
      t.position = { world_position.x, world_position.y, 0 };

      r.emplace<SpriteComponent>(e, s);
      r.emplace<TransformComponent>(e, t);
      r.emplace<SpriteColourComponent>(e, scc);

      // TODO: improve lines of code below
      std::vector<entt::entity> entities = grid_entities_at(r, grid_index.x, grid_index.y);
      for (const auto& entity : entities)
        r.destroy(entity);
      r.emplace<GridComponent>(e, grid_index.x, grid_index.y);
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
    r.emplace<GridComponent>(e, x, y);
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
  PhysicsTransformComponent p0;
  p0.x_tl = r0.x1;
  p0.y_tl = r0.y1;
  p0.w = r0.w;
  p0.h = r0.h;

  PhysicsTransformComponent p1;
  p1.x_tl = r1.x1;
  p1.y_tl = r1.y1;
  p1.w = r1.w;
  p1.h = r1.h;

  return collide(p0, p1);
};

constexpr glm::ivec2
room_center(const Room& r)
{
  return { (r.x1 + r.x2) / 2, (r.y1 + r.y2) / 2 };
};

//
// entry functions
//

void
generate_dungeon(entt::registry& r, const Dungeon& d, int step)
{
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();

  // destroy any grid tiles
  const auto& view_grid_entities = r.view<const GridComponent>();
  view_grid_entities.each([&r](auto entity, const auto& grid) { r.destroy(entity); });

  int offset_x = 0;
  int offset_y = 0;

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
      r.emplace<GridComponent>(e, grid_index.x, grid_index.y);

      // if (x != 0 && y != 0 && x != d.width - 1 && y != d.height - 1)
      //   r.emplace<HealthComponent>(e, 1, 1); // give inner walls health

      r.emplace<SpriteColourComponent>(e, scc);
    }
  }

  engine::RandomState rnd;
  rnd.rng.seed(0);
  const int room_min_size = 6;
  const int room_max_size = 10;
  const int max_rooms = 30;
  const int max_monsters_per_room = 5;

  std::vector<Room> rooms;

  if (step == -1)
    step = max_rooms + 1;
  std::cout << "step is: " << step << "max is: " << max_rooms << "\n";

  for (int max_room_idx = 0; max_room_idx < max_rooms; max_room_idx++) {
    if (max_room_idx > step)
      break;

    int room_width = static_cast<int>(engine::rand_det_s(rnd.rng, room_min_size, room_max_size));
    int room_height = static_cast<int>(engine::rand_det_s(rnd.rng, room_min_size, room_max_size));
    int x = static_cast<int>(engine::rand_det_s(rnd.rng, 0, d.width - room_width - 1));
    int y = static_cast<int>(engine::rand_det_s(rnd.rng, 0, d.height - room_height - 1));

    Room room;
    room.x1 = offset_x + x;
    room.y1 = offset_y + y;
    room.x2 = room.x1 + room_width;
    room.y2 = room.y1 + room_height;
    room.w = room_width;
    room.h = room_height;

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
  // Set pathfinding cost
  //
  {
    const auto& grid_tiles = r.view<const GridComponent>();
    grid_tiles.each([&r](auto entity, const auto& grid) {
      EntityTypeComponent& t = r.get<EntityTypeComponent>(entity);

      PathfindableComponent path;

      if (t.type == EntityType::floor)
        path.cost = 0;
      else if (t.type == EntityType::wall)
        path.cost = -1; // impassable
      else
        path.cost = 1;

      r.emplace<PathfindableComponent>(entity, path);
    });
  }

  //
  // Gameplay logic
  // Put a player in a room
  // limitation: currently all player put in same spot
  {
    const auto& view = r.view<TransformComponent, const PlayerComponent>();
    view.each([&rooms](auto entity, TransformComponent& t, const PlayerComponent& p) {
      if (rooms.size() > 0) {
        auto room = rooms[0];
        auto center = room_center(room);
        glm::ivec2 pos = engine::grid::grid_space_to_world_space(center, GRID_SIZE);
        t.position = { pos.x, pos.y, 0 };
      }
    });
  }

  //
  // Gameplay logic
  // placing enemies
  //
  for (const auto& room : rooms) {
    // int number_of_monsters = static_cast<int>(engine::rand_det_s(rnd.rng, 0, max_monsters_per_room));
    int number_of_monsters = static_cast<int>(engine::rand_det_s(rnd.rng, 0, max_monsters_per_room));

    std::vector<glm::ivec2> room_occupied_slots;

    for (int i = 0; i < number_of_monsters; i++) {

      float random = engine::rand_det_s(rnd.rng, 0.0f, 1.0f);
      EntityType et = EntityType::enemy_orc;

      if (random < 0.8f)
        et = EntityType::enemy_orc;
      else
        et = EntityType::enemy_troll;

      int x = static_cast<int>(engine::rand_det_s(rnd.rng, room.x1 + 1, room.x2 - 1));
      int y = static_cast<int>(engine::rand_det_s(rnd.rng, room.y1 + 1, room.y2 - 1));
      glm::ivec2 grid_index = { x, y };

      entt::entity e = create_gameplay(r, et);
      SpriteComponent s = create_sprite(r, e, et);
      TransformComponent t = create_transform(r, e);
      SpriteColourComponent scc = create_colour(r, e, et);

      // Check the tile isn't occupied
      auto full = std::find_if(room_occupied_slots.begin(), room_occupied_slots.end(), [&grid_index](const auto& val) {
        return grid_index == val;
      });
      if (full != room_occupied_slots.end()) {
        printf("already entity at position");
        return;
      }
      room_occupied_slots.push_back(grid_index);

      // Create the entity
      glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);
      t.position = { world_position.x, world_position.y, 0 };
      r.emplace<TransformComponent>(e, t);
      r.emplace<SpriteComponent>(e, s);
      r.emplace<SpriteColourComponent>(e, scc);
      r.emplace<GridComponent>(e, grid_index.x, grid_index.y);

      // randomize brain offset time to prevent fps drops
      // auto& brain = r.get<AiBrainComponent>(e);
      // brain.milliseconds_between_ai_updates_left = engine::rand_det_s(rnd.rng, 0, k_milliseconds_between_ai_updates);
    }
  }

  // Put a shopkeeper in a room
  // const auto& shopkeeper_view = r.view<ShopKeeperComponent, TransformComponent>();
  // shopkeeper_view.each([&rooms](ShopKeeperComponent& sk, TransformComponent& t) {
  //   if (rooms.size() > 1) {
  //     auto room = rooms[1];
  //     auto center = room_center(room);
  //     glm::ivec2 pos = engine::grid::grid_space_to_world_space(center, GRID_SIZE);
  //     t.position = { pos.x, pos.y, 0 };
  //   }
  // });
};

static bool first_frame = true;

void
update_dungeon_system(entt::registry& r)
{
  if (first_frame) {
    first_frame = false;
    return;
  }
  // glm::ivec2 grid_space_center = room_center(rooms[0]);
  // glm::vec2 world_space_center = engine::grid::grid_space_to_world_space(grid_space_center, GRID_SIZE);

  // center the camera on the center of the room
  // const auto& ri = r.ctx().at<SINGLETON_RendererInfo>();
  // const auto& cameras = r.view<CameraComponent, TransformComponent>();
  // for (auto [entity, camera, transform] : cameras.each()) {
  //   transform.position.x = (-ri.viewport_size_render_at.x / 2) + world_space_center.x;
  //   transform.position.y = (-ri.viewport_size_render_at.y / 2) + world_space_center.y;
  // };
}

} // namespace game2d