#include "helpers.hpp"

#include "engine/maths/grid.hpp"
#include "game/helpers/line.hpp"
#include "game/modules/ai/components.hpp"
#include "game/modules/fov/components.hpp"
#include "game/modules/player/components.hpp"
#include "modules/physics/helpers.hpp"

namespace game2d {

const int GRID_SIZE = 16;

} // namespace game2d

bool
game2d::rooms_overlap(const Room& r0, const Room& r1)
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

std::vector<entt::entity>
game2d::grid_entities_at(entt::registry& r, int x, int y)
{
  std::vector<entt::entity> results;
  const auto& view = r.view<const GridComponent>();
  view.each([&results, &x, &y](auto entity, const auto& grid) {
    if (x == grid.x && y == grid.y)
      results.push_back(entity);
  });
  return results;
};

void
game2d::get_neighbour_indicies(const int x,
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
game2d::create_room(GameEditor& editor, Game& game, const Room& room)
{
  auto& r = game.state;

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

      entt::entity e = create_gameplay(editor, game, et);
      SpriteComponent s = create_sprite(editor, r, e, et);
      TransformComponent t = create_transform(r, e);
      SpriteColourComponent scc = create_colour(editor, r, e, et);

      glm::ivec2 grid_index = { room.x1 + x, room.y1 + y };
      glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);
      t.position = { world_position.x, world_position.y, 0 };

      r.emplace<SpriteComponent>(e, s);
      r.emplace<TransformComponent>(e, t);
      r.emplace<SpriteColourComponent>(e, scc);
      r.emplace<FovComponent>(e);

      // TODO: improve lines of code below
      std::vector<entt::entity> entities = grid_entities_at(r, grid_index.x, grid_index.y);
      for (const auto& entity : entities)
        r.destroy(entity);
      r.emplace<GridComponent>(e, grid_index.x, grid_index.y);
    }
  }
};

void
game2d::create_tunnel_floor(GameEditor& editor, Game& game, const Dungeon& d, std::vector<std::pair<int, int>>& coords)
{
  auto& r = game.state;

  for (const auto& coord : coords) {
    int x = coord.first;
    int y = coord.second;

    EntityType et = EntityType::floor;
    entt::entity e = create_gameplay(editor, game, et);
    SpriteComponent s = create_sprite(editor, r, e, et);
    TransformComponent t = create_transform(r, e);
    SpriteColourComponent scc = create_colour(editor, r, e, et);

    glm::ivec2 pos = engine::grid::grid_space_to_world_space({ x, y }, GRID_SIZE);
    t.position = { pos.x, pos.y, 0 };

    r.emplace<SpriteComponent>(e, s);
    r.emplace<TransformComponent>(e, t);
    r.emplace<SpriteColourComponent>(e, scc);
    r.emplace<FovComponent>(e);

    std::vector<entt::entity> entities = grid_entities_at(r, x, y);
    for (const auto& entity : entities)
      r.destroy(entity);
    r.emplace<GridComponent>(e, x, y);
  }
};

void
game2d::create_tunnel(GameEditor& editor, Game& game, const Dungeon& d, int x1, int y1, int x2, int y2)
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

void
game2d::set_pathfinding_cost(GameEditor& editor, Game& game)
{
  auto& r = game.state;
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
};

entt::entity
game2d::create_dungeon_entity(GameEditor& editor, Game& game, EntityType et, glm::ivec2 grid_index)
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
  r.emplace<GridComponent>(e, grid_index.x, grid_index.y);
  r.emplace<FovComponent>(e);

  return e;
}

void
game2d::set_player_positions(GameEditor& editor, Game& game, std::vector<Room>& rooms, engine::RandomState& rnd)
{
  auto& r = game.state;
  const auto& view = r.view<TransformComponent, const PlayerComponent>();
  view.each([&rooms](auto entity, TransformComponent& t, const PlayerComponent& p) {
    if (rooms.size() > 0) {
      auto room = rooms[0];
      auto center = room_center(room);
      glm::ivec2 pos = engine::grid::grid_space_to_world_space(center, GRID_SIZE);
      t.position = { pos.x, pos.y, 0 };
    }
  });
};

void
game2d::set_enemy_positions(GameEditor& editor, Game& game, std::vector<Room>& rooms, engine::RandomState& rnd)
{
  constexpr int max_monsters_per_room = 5;

  for (auto& room : rooms) {
    int number_of_monsters = static_cast<int>(engine::rand_det_s(rnd.rng, 0, max_monsters_per_room));
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

      // Check the tile isn't occupied
      auto full = std::find_if(
        room.occupied.begin(), room.occupied.end(), [&grid_index](const auto& val) { return grid_index == val; });
      if (full != room.occupied.end()) {
        printf("already entity at position");
        continue;
      }

      room.occupied.push_back(grid_index);

      // randomize brain offset time to prevent fps drops
      auto e = create_dungeon_entity(editor, game, et, grid_index);
      auto& r = game.state;
      auto& brain = r.get<AiBrainComponent>(e);
      brain.milliseconds_between_ai_updates_left = engine::rand_det_s(rnd.rng, 0, k_milliseconds_between_ai_updates);
    }
  }
}

void
game2d::set_item_positions(GameEditor& editor, Game& game, std::vector<Room>& rooms, engine::RandomState& rnd)
{
  constexpr int max_items_per_room = 4;

  for (auto& room : rooms) {
    int number_of_items = engine::rand_det_s(rnd.rng, 0, max_items_per_room);

    for (int i = 0; i < number_of_items; i++) {
      int x = static_cast<int>(engine::rand_det_s(rnd.rng, room.x1 + 1, room.x2 - 1));
      int y = static_cast<int>(engine::rand_det_s(rnd.rng, room.y1 + 1, room.y2 - 1));
      glm::ivec2 grid_index = { x, y };
      glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);

      // Check the tile isn't occupied
      auto full = std::find_if(
        room.occupied.begin(), room.occupied.end(), [&grid_index](const auto& val) { return grid_index == val; });
      if (full != room.occupied.end()) {
        printf("already entity at position");
        continue;
      }
      room.occupied.push_back(grid_index);

      float percent = engine::rand_det_s(rnd.rng, 0.0f, 1.0f);
      if (percent < 0.7f)
        create_dungeon_entity(editor, game, EntityType::potion, grid_index);
      else
        create_dungeon_entity(editor, game, EntityType::scroll_damage_nearest, grid_index);
    }
  }
};
