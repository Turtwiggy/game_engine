#include "system.hpp"

#include "components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
#include "game/modules/ai/components.hpp"
#include "game/modules/fov/components.hpp"
#include "game/modules/player/components.hpp"
#include "helpers.hpp"
#include "modules/camera/components.hpp"

#include <map>

namespace game2d {

static const int GRID_SIZE = 16;

void
generate_dungeon(GameEditor& editor, Game& game, const Dungeon& d)
{
  const auto& colours = editor.colours;
  auto& r = game.state;

  int offset_x = 0;
  int offset_y = 0;

  // create all the tiles
  for (int x = 0; x < d.width; x++) {
    for (int y = 0; y < d.height; y++) {
      EntityType et = EntityType::wall;

      entt::entity e = create_gameplay(editor, game, et);
      SpriteComponent s = create_sprite(editor, r, e, et);
      TransformComponent t = create_transform(r, e);
      SpriteColourComponent scc = create_colour(editor, r, e, et);

      glm::ivec2 grid_index = { offset_x + x, offset_y + y };
      glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);
      t.position = { world_position.x, world_position.y, 0 };

      r.emplace<SpriteComponent>(e, s);
      r.emplace<TransformComponent>(e, t);
      r.emplace<GridComponent>(e, grid_index.x, grid_index.y);
      r.emplace<FovComponent>(e);

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

  // if (step == -1)
  //   step = max_rooms + 1;
  // std::cout << "step is: " << step << "max is: " << max_rooms << "\n";

  for (int max_room_idx = 0; max_room_idx < max_rooms; max_room_idx++) {
    // if (max_room_idx > step)
    //   break;

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

    create_room(editor, game, room);

    // dig out a tunnel between this room and the previous one
    bool starting_room = max_room_idx == 0;
    if (!starting_room) {
      auto r0_center = room_center(rooms[rooms.size() - 1]);
      auto r1_center = room_center(room);
      create_tunnel(editor, game, d, r0_center.x, r0_center.y, r1_center.x, r1_center.y);
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

      entt::entity e = create_gameplay(editor, game, et);
      SpriteComponent s = create_sprite(editor, r, e, et);
      TransformComponent t = create_transform(r, e);
      SpriteColourComponent scc = create_colour(editor, r, e, et);

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
      r.emplace<FovComponent>(e);

      // randomize brain offset time to prevent fps drops
      auto& brain = r.get<AiBrainComponent>(e);
      brain.milliseconds_between_ai_updates_left = engine::rand_det_s(rnd.rng, 0, k_milliseconds_between_ai_updates);
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

void
update_dungeon_system(GameEditor& editor, Game& game)
{
  if (game.on_start.Get()) {
    const int room_width = 50;
    const int room_height = 50;
    Room room;
    room.x1 = 0;
    room.y1 = 0;
    room.x2 = room_width;
    room.y2 = room_height;
    room.w = room_width;
    room.h = room_height;

    glm::ivec2 grid_space_center = room_center(room);
    glm::vec2 world_space_center = engine::grid::grid_space_to_world_space(grid_space_center, GRID_SIZE);

    // center the camera on the center of the room
    auto& r = game.state;
    const auto& ri = editor.renderer;

    const auto& cameras = r.view<CameraComponent, TransformComponent>();
    for (auto [entity, camera, transform] : cameras.each()) {
      transform.position.x = (-ri.viewport_size_render_at.x / 2) + world_space_center.x;
      transform.position.y = (-ri.viewport_size_render_at.y / 2) + world_space_center.y;
    };
  }
  game.on_start.Set(false);
}

} // namespace game2d