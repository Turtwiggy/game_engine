#include "helpers.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/helpers/line.hpp"
#include "game/modules/ai/components.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/dungeon/helpers.hpp"
#include "game/modules/fov/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "game/modules/rpg_xp/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/ui_hierarchy/helpers.hpp"

#include "magic_enum.hpp"

#include <vector>

namespace game2d {

const int GRID_SIZE = 16;

std::vector<entt::entity>
grid_entities_at(Game& game, int x, int y)
{
  auto& r = game.state;
  std::vector<entt::entity> results;
  const auto& view = r.view<const GridComponent>();
  view.each([&results, &x, &y](auto entity, const auto& grid) {
    if (x == grid.x && y == grid.y)
      results.push_back(entity);
  });
  return results;
};

entt::entity
create_dungeon_entity_remove_old(GameEditor& editor, Game& game, EntityType et, const glm::ivec2& grid_index)
{
  auto& r = game.state;
  std::vector<entt::entity> entities = grid_entities_at(game, grid_index.x, grid_index.y);
  for (const auto& entity : entities)
    r.destroy(entity);

  return create_dungeon_entity(editor, game, et, grid_index);
};

} // namespace game2d

entt::entity
game2d::create_dungeon_entity(GameEditor& editor, Game& game, EntityType et, const glm::ivec2& grid_index)
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

void
game2d::create_room(GameEditor& editor, Game& game, const Room& room)
{
  for (int x = 0; x < room.w; x++) {
    for (int y = 0; y < room.h; y++) {

      EntityType et = EntityType::tile_type_floor;
      if (x == 0)
        et = EntityType::tile_type_wall;
      if (y == 0)
        et = EntityType::tile_type_wall;
      if (x == room.w - 1)
        et = EntityType::tile_type_wall;
      if (y == room.h - 1)
        et = EntityType::tile_type_wall;

      const glm::ivec2 grid_index = { room.x1 + x, room.y1 + y };
      if (EntityType::tile_type_floor == et)
        create_dungeon_entity_remove_old(editor, game, et, grid_index);
    }
  }
};

void
game2d::create_tunnel_floor(GameEditor& editor, Game& game, const Dungeon& d, std::vector<std::pair<int, int>>& coords)
{
  for (const auto& coord : coords)
    create_dungeon_entity_remove_old(editor, game, EntityType::tile_type_floor, { coord.first, coord.second });
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
  const auto& view = r.view<const GridComponent, const EntityTypeComponent>();
  for (auto [entity, grid, et] : view.each()) {
    PathfindableComponent path;

    if (et.type == EntityType::tile_type_floor)
      path.cost = 0;
    else if (et.type == EntityType::tile_type_wall)
      path.cost = -1; // impassable
    else
      path.cost = 1;

    r.emplace<PathfindableComponent>(entity, path);
  }
};

void
game2d::set_player_positions(GameEditor& editor, Game& game, std::vector<Room>& rooms, engine::RandomState& rnd)
{
  auto& r = game.state;
  const auto& view = r.view<TransformComponent, const PlayerComponent>();
  view.each([&rooms, &game, &r](auto entity, TransformComponent& t, const PlayerComponent& p) {
    if (rooms.size() > 0) {
      auto room = rooms[0];
      auto center = room_center(room);

      // forceapply player
      auto entities = grid_entities_at(game, center.x, center.y);
      for (const auto& e : entities) {
        const auto& type = r.get<EntityTypeComponent>(e);
        const auto& entity_type = type.type;
        std::string value_str = std::string(magic_enum::enum_name(entity_type));

        if (entity_type == EntityType::tile_type_exit)
          continue;
        if (entity_type == EntityType::tile_type_floor)
          continue;
        if (entity_type == EntityType::tile_type_wall)
          continue;

        std::cout << "player deleted generated entity " << value_str << " as it was spawned on it\n";
        r.destroy(e);
      }

      glm::ivec2 pos = engine::grid::grid_space_to_world_space(center, GRID_SIZE);
      t.position = { pos.x, pos.y, 0 };
    }
  });
};

namespace game2d {

EntityType
generate_item(const int floor, engine::RandomState& rnd)
{
  const std::vector<std::pair<int, std::pair<EntityType, int>>> difficulty_map{
    // clang-format off
    { 0, { EntityType::potion, 80 } },
    { 0, { EntityType::sword, 15 } },
    { 0, { EntityType::shield, 15 } },
    { 1, { EntityType::scroll_damage_nearest, 5 } },
    { 1, { EntityType::scroll_damage_selected_on_grid, 5 } },
    { 2, { EntityType::crossbow, 15 } },
    { 2, { EntityType::bolt, 15 } },
    // clang-format on
  };

  std::map<EntityType, int> weights;
  for (const auto& [dict_floor, dict_type] : difficulty_map) {

    // only interested in weights at our floor
    if (dict_floor > floor)
      break;

    // sum the weights!
    const auto& entity_type = dict_type.first;
    const auto& entity_weight = dict_type.second;
    if (weights.contains(entity_type))
      weights[entity_type] += entity_weight;
    else
      weights[entity_type] = entity_weight;
  }

  int total_weight = 0;
  for (const auto& [k, w] : weights)
    total_weight += w;

  const float random = engine::rand_det_s(rnd.rng, 0, total_weight);
  int weight_acculum = 0;
  for (const auto& [k, w] : weights) {
    weight_acculum += w;
    if (weight_acculum >= random)
      return k;
  }
  return EntityType::potion;
};

EntityType
generate_monster(const int floor, engine::RandomState& rnd)
{
  const std::vector<std::pair<int, std::pair<EntityType, int>>> difficulty_map{
    // clang-format off
    { 0, { EntityType::actor_orc, 80 } },
    { 2, { EntityType::actor_troll, 15 } },
    { 4, { EntityType::actor_troll, 30 } },
    { 6, { EntityType::actor_troll, 60 } }
    // clang-format on
  };

  std::map<EntityType, int> weights;
  for (const auto& [dict_floor, dict_type] : difficulty_map) {

    // only interested in weights at our floor
    if (dict_floor > floor)
      break;

    // sum the weights!
    const auto& entity_type = dict_type.first;
    const auto& entity_weight = dict_type.second;
    if (weights.contains(entity_type))
      weights[entity_type] += entity_weight;
    else
      weights[entity_type] = entity_weight;
  }

  int total_weight = 0;
  for (const auto& [k, w] : weights)
    total_weight += w;

  const float random = engine::rand_det_s(rnd.rng, 0, total_weight);
  int weight_acculum = 0;
  for (const auto& [k, w] : weights) {
    weight_acculum += w;
    if (weight_acculum >= random)
      return k;
  }
  return EntityType::actor_orc;
};

} // namespace game2d

void
game2d::set_generated_entity_positions(GameEditor& editor,
                                       Game& game,
                                       std::vector<Room>& rooms,
                                       const int floor,
                                       engine::RandomState& rnd)
{
  auto& r = game.state;
  // randomize things
  const int amount_of_rooms = rooms.size();
  const int potential_items_per_room = 5 + (floor);
  const int potential_monsters_per_room = 5 + (floor);
  std::cout << "floor " << floor << " has max items: " << potential_items_per_room
            << ", max monsters:" << potential_monsters_per_room << "\n";

  for (int j = 0; auto& room : rooms) {
    const int number_of_items = engine::rand_det_s(rnd.rng, 0, potential_items_per_room);
    const int number_of_monsters = engine::rand_det_s(rnd.rng, 0, potential_monsters_per_room);
    std::cout << "room " << j << " has " << number_of_items << " items, " << number_of_monsters << " monsters\n";
    j++;

    std::vector<EntityType> generated;
    for (int i = 0; i < number_of_items; i++)
      generated.push_back(generate_item(floor, rnd)); // randomize rarity
    for (int i = 0; i < number_of_monsters; i++)
      generated.push_back(generate_monster(floor, rnd)); // randomize rarity

    //
    // place generated entities
    //
    for (int i = 0; i < generated.size(); i++) {

      const int x = static_cast<int>(engine::rand_det_s(rnd.rng, room.x1 + 1, room.x2 - 1));
      const int y = static_cast<int>(engine::rand_det_s(rnd.rng, room.y1 + 1, room.y2 - 1));
      const glm::ivec2 grid_index = { x, y };

      // Check the tile isn't occupied
      const auto full = std::find_if(
        room.occupied.begin(), room.occupied.end(), [&grid_index](const auto& val) { return grid_index == val; });
      if (full != room.occupied.end())
        continue; // entity already at position

      room.occupied.push_back(grid_index);

      create_dungeon_entity(editor, game, generated[i], grid_index);
    }
  }
}

entt::entity
game2d::generate_dungeon(GameEditor& editor, Game& game, const Dungeon& d, uint32_t seed)
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

  auto& r = game.state;
  entt::entity e = r.create();
  r.emplace<EntityTypeComponent>(e, EntityType::empty);
  r.emplace<TagComponent>(e, "dungeon");
  r.emplace<Dungeon>(e, d);
  return e;
};

namespace game2d {

void
transfer_old_state(GameEditor& editor, Game& game)
{
  // old gamestate of interest...
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
};

} // namespace game2d

// reset game state. this is because I could not seem to
// correctly clear the registry AND use entt groups at the same time.
entt::entity
game2d::generate_dungeon_transfer_old_state(GameEditor& editor, Game& game, uint32_t& seed)
{
  transfer_old_state(editor, game);

  seed += 1;

  Dungeon d; // set dungeon specs
  d.seed = seed;
  d.floor = seed;
  std::string floor_msg = "Exit reached! New floor:" + std::to_string(d.floor);
  game.ui_events.events.push_back(floor_msg);

  std::string msg = "Generating new dungeon! seed: " + std::to_string(seed);
  game.ui_events.events.push_back(msg);
  return generate_dungeon(editor, game, d, seed);
}