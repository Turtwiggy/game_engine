#include "system.hpp"

#include "components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/modules/ai/components.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/dungeon/helpers.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "game/modules/rpg_xp/components.hpp"
#include "helpers.hpp"
#include "modules/camera/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"

namespace game2d {

static const int GRID_SIZE = 16;

void
generate_dungeon(GameEditor& editor, Game& game, const Dungeon& d, uint32_t seed)
{
  const auto& colours = editor.colours;

  int offset_x = 0;
  int offset_y = 0;

  // create all tiles as walls
  for (int x = 0; x < d.width; x++) {
    for (int y = 0; y < d.height; y++) {
      const glm::ivec2 grid_index = { offset_x + x, offset_y + y };
      create_dungeon_entity(editor, game, EntityType::tile_type_wall, grid_index);
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
  // Steps after initial initialization...
  //
  const int floor = d.floor;
  set_pathfinding_cost(editor, game);
  set_generated_entity_positions(editor, game, rooms, floor, rnd);
  // limitation: currently all player put in same spot
  set_player_positions(editor, game, rooms, rnd);

  // set exit door position
  // bug: exit placed on generated entities. not so bad?
  Room& room = rooms[rooms.size() - 1];
  glm::ivec2 middle = room_center(room);
  create_dungeon_entity(editor, game, EntityType::tile_type_exit, middle);
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

  // argh... copy out gamestate of interest...
  std::vector<HealthComponent> old_hp;
  std::vector<XpComponent> old_xp;
  std::vector<StatsComponent> old_stats;
  std::vector<std::pair<IsEquipped, EntityType>> old_equipped_items;
  std::vector<EntityType> old_items;
  {
    auto& old_r = game.state;
    // ... player data
    for (const auto [entity_player, player, xp, stats, type, hp] :
         old_r.view<PlayerComponent, XpComponent, StatsComponent, EntityTypeComponent, HealthComponent>().each()) {
      old_xp.push_back(xp);
      old_stats.push_back(stats);
      old_hp.push_back(hp);

      // ... equipped items
      for (const auto [item_eqp, item, type] : old_r.view<IsEquipped, EntityTypeComponent>().each()) {
        if (item.parent != entity_player)
          continue; // not player equipment
        old_equipped_items.push_back({ item, type.type });
      }

      // ... items in backpack
      for (const auto [item_ibp, item, type] : old_r.view<InBackpackComponent, EntityTypeComponent>().each()) {
        if (item.parent != entity_player)
          continue; // not a player item
        old_items.push_back(type.type);
      }
    }
  }

  game.state = entt::registry();
  auto& r = game.state;

  // TODO: copy out any components that want persisting across dungeons?
  // e.g. xp, player inventory?

  // WARNING: below code is duplicate of game.cpp
  // --------------------------------------------

  create_hierarchy_root_node(r);

  // players
  for (int i = 0; i < 1; i++) {
    EntityType et = EntityType::actor_player;
    entt::entity e = create_gameplay(editor, game, et);
    create_renderable(editor, r, e, et);
  }

  // ... copy over stats
  for (int i = 0; const auto [entity, player, xp, stats, type, hp] :
                  r.view<PlayerComponent, XpComponent, StatsComponent, EntityTypeComponent, HealthComponent>().each()) {
    xp.amount = old_xp[i].amount;
    stats.agi_level = old_stats[i].agi_level;
    stats.con_level = old_stats[i].con_level;
    stats.str_level = old_stats[i].str_level;
    stats.overall_level = old_stats[i].overall_level;
    hp.hp = old_hp[i].hp;

    // ... give player items
    std::cout << "items: " << old_items.size();
    for (const auto& item : old_items)
      create_item(editor, game, item, entity);

    // ... equip the same items
    WantsToEquip& equip = r.get_or_emplace<WantsToEquip>(entity);
    for (const auto& [eitem, type] : old_equipped_items) {
      auto new_item = create_item(editor, game, type, entity);
      equip.requests.push_back({ eitem.slot, new_item });
    }

    break; // limit to one player for the moment
    i++;
  }

  // camera
  auto c = create_gameplay(editor, game, EntityType::camera);
  r.emplace<TransformComponent>(c);

  static int seed = 1;
  seed += 1;

  Dungeon d; // set dungeon specs
  d.seed = seed;
  d.floor = seed;
  std::string floor_msg = "Exit reached! New floor:" + std::to_string(d.floor);
  game.ui_events.events.push_back(floor_msg);

  if (d.floor == 10) {
    game.ui_events.events.push_back("You win!");
    game.gameover = true;
  }

  std::string msg = "Generating new dungeon! seed: " + std::to_string(seed);
  game.ui_events.events.push_back(msg);
  generate_dungeon(editor, game, d, seed);

  entt::entity e = r.create();
  r.emplace<EntityTypeComponent>(e, EntityType::empty);
  r.emplace<TagComponent>(e, "dungeon");
  r.emplace<Dungeon>(e, d);
};

} // namespace game2d