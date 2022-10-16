#include "system.hpp"

#include "components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/modules/ai/components.hpp"
#include "game/modules/dungeon/helpers.hpp"
#include "game/modules/player/components.hpp"
#include "helpers.hpp"
#include "modules/camera/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"

#include <format>

namespace game2d {

static const int GRID_SIZE = 16;

void
generate_dungeon(GameEditor& editor, entt::registry& r, const Dungeon& d, uint32_t seed)
{
  const auto& colours = editor.colours;

  int offset_x = 0;
  int offset_y = 0;

  // create all tiles as walls
  for (int x = 0; x < d.width; x++) {
    for (int y = 0; y < d.height; y++) {
      const glm::ivec2 grid_index = { offset_x + x, offset_y + y };
      create_dungeon_entity(editor, r, EntityType::wall, grid_index);

      // if (x != 0 && y != 0 && x != d.width - 1 && y != d.height - 1)
      //   r.emplace<HealthComponent>(e, 1, 1); // give inner walls health
    }
  }

  engine::RandomState rnd;
  rnd.rng.seed(seed);
  constexpr int room_min_size = 6;
  constexpr int room_max_size = 10;
  constexpr int max_rooms = 30;

  std::vector<Room> rooms;

  for (int max_room_idx = 0; max_room_idx < max_rooms; max_room_idx++) {
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

    create_room(editor, r, room);

    // dig out a tunnel between this room and the previous one
    bool starting_room = max_room_idx == 0;
    if (!starting_room) {
      auto r0_center = room_center(rooms[rooms.size() - 1]);
      auto r1_center = room_center(room);
      create_tunnel(editor, r, d, r0_center.x, r0_center.y, r1_center.x, r1_center.y);
    }

    rooms.push_back(room);
  }

  //
  // Steps after initial initialization...
  //
  set_pathfinding_cost(editor, r);
  set_player_positions(editor, r, rooms, rnd); // limitation: currently all player put in same spot
  set_enemy_positions(editor, r, rooms, rnd);
  set_item_positions(editor, r, rooms, rnd);

  // set exit door position
  Room& room = rooms[rooms.size() - 1];
  glm::ivec2 middle = room_center(room);
  create_dungeon_entity(editor, r, EntityType::exit, middle);
};

void
update_dungeon_system(GameEditor& editor, Game& game)
{
  {
    auto& r = game.state;
    const auto& view = r.view<const PlayerComponent, const CollidingWithExitComponent>();
    bool collision_occured = view.size_hint() > 0;
    if (!collision_occured)
      return; // not interested
  }

  // reset game state. this is because I could not seem to
  // correctly clear the registry AND use entt groups at the same time.
  game.state = entt::registry();
  auto& r = game.state;

  // TODO: copy out any components that want persisting across dungeons?
  // e.g. xp, player inventory?

  // WARNING: below code is duplicate of game.cpp
  // --------------------------------------------

  create_hierarchy_root_node(r);

  // players
  for (int i = 0; i < 1; i++) {
    EntityType et = EntityType::player;
    entt::entity e = create_gameplay(editor, r, et);
    create_renderable(editor, r, e, et);
  }

  // camera
  auto c = create_gameplay(editor, r, EntityType::camera);
  r.emplace<TransformComponent>(c);

  static int seed = 1;
  seed += 1;
  std::string msg = std::format("New dungeon! seed: {}", seed);
  game.ui_events.events.push_back(msg);

  Dungeon d; // set dungeon specs
  d.seed = seed;
  generate_dungeon(editor, r, d, seed);
  entt::entity e = r.create();
  r.emplace<EntityTypeComponent>(e, EntityType::empty);
  r.emplace<TagComponent>(e, "dungeon");
  r.emplace<Dungeon>(e, d);
};

} // namespace game2d