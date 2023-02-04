#include "helpers.hpp"

#include "camera/components.hpp"
#include "components.hpp"
#include "components/actors.hpp"
#include "components/events.hpp"
#include "events/system.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/combat/components.hpp"
#include "modules/dungeon/helpers/create.hpp"
#include "modules/dungeon/helpers/generate.hpp"
#include "modules/dungeon/helpers/rooms.hpp"
#include "modules/items/components.hpp"
#include "modules/player/components.hpp"
#include "modules/rpg_xp/components.hpp"

// headers that probably shouldnt be in this file
#include "modules/ai/components.hpp"
#include "modules/ai/helpers.hpp"
#include "modules/fov/system.hpp"
#include "modules/ui_hierarchy/helpers.hpp"

#include "magic_enum.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <string>
#include <utility>
#include <vector>

namespace game2d {

const int GRID_SIZE = 16;

void
set_player_positions(GameEditor& editor, Game& game, Dungeon& d, engine::RandomState& rnd)
{
  auto& r = game.state;
  for (const auto [entity, transform, player] : r.view<TransformComponent, PlayerComponent>().each()) {
    if (d.rooms.size() == 0)
      continue;
    const auto& room = d.rooms[0];
    const auto center = room_center(room);
    glm::ivec2 pos = engine::grid::grid_space_to_world_space(center, GRID_SIZE);

    // Fixes: placing player on generated entities.
    const auto it =
      std::find_if(d.occupied.begin(), d.occupied.end(), [&pos](const std::pair<entt::entity, glm::ivec2>& other) {
        return pos.x == other.second.x && pos.y == other.second.y;
      });
    if (it != d.occupied.end()) {
      printf("player spawned on generated something, deleting generated something");
      r.destroy((*it).first);
    }

    transform.position = { pos.x, pos.y, 0 };
  }
};

void
set_generated_entity_positions(GameEditor& editor, Game& game, Dungeon& d, engine::RandomState& rnd)
{
  auto& r = game.state;

  // randomize things
  const int floor = d.floor;
  const int amount_of_rooms = d.rooms.size();
  const int potential_items_per_room = 5 + (floor);
  const int potential_monsters_per_room = 5 + (floor);

  printf("floor %i has max items %i, max monsters: %i\n", floor, potential_items_per_room, potential_monsters_per_room);

  for (int j = 0; const auto& room : d.rooms) {
    const int number_of_items = engine::rand_det_s(rnd.rng, 0, potential_items_per_room);
    const int number_of_monsters = engine::rand_det_s(rnd.rng, 0, potential_monsters_per_room);
    std::cout << "room " << j << " has " << number_of_items << " items, " << number_of_monsters << " monsters\n";
    j++;

    std::vector<EntityType> generated;
    for (int i = 0; i < number_of_items; i++)
      generated.push_back(generate_item(floor, rnd)); // randomize rarity
    for (int i = 0; i < number_of_monsters; i++)
      generated.push_back(generate_monster(floor, rnd)); // randomize rarity

    // place generated entities
    for (int i = 0; i < generated.size(); i++) {
      const int x = static_cast<int>(engine::rand_det_s(rnd.rng, room.x1 + 1, room.x2 - 1));
      const int y = static_cast<int>(engine::rand_det_s(rnd.rng, room.y1 + 1, room.y2 - 1));
      const glm::ivec2 grid_index = { x, y };

      // Check the tile isn't occupied
      const auto full = std::find_if(
        d.occupied.begin(), d.occupied.end(), [&grid_index](const std::pair<entt::entity, glm::ivec2>& other) {
          return other.second == grid_index;
        });
      if (full != d.occupied.end())
        continue; // entity already at position

      auto entity = create_dungeon_entity(editor, game, generated[i], grid_index);

      // increase the generated monsters items stats per floor
      if (auto* stats = r.try_get<StatsComponent>(entity)) {
        // stats->str_level += (floor / 3);
        // stats->agi_level += (floor / 3);
        stats->con_level += (floor / 2);
        printf("new monsters stats: (con)%i (str)%i (agi)%i\n", stats->con_level, stats->str_level, stats->agi_level);
      }

      d.occupied.push_back({ entity, grid_index });
    }
  }
}

void
generate_dungeon(GameEditor& editor, Game& game, const int size_x, const int size_y, const int seed, const int floor)
{
  const auto& colours = editor.colours;
  auto& r = game.state;

  Dungeon d;
  d.width = size_x;
  d.height = size_y;
  d.floor = floor;
  d.walls_and_floors.resize(d.width * d.height);

  engine::RandomState rnd;
  rnd.rng.seed(seed);
  create_all_walls(editor, game, d);
  create_all_rooms(editor, game, d, rnd);

  // Steps after initial initialization...
  set_generated_entity_positions(editor, game, d, rnd);
  set_player_positions(editor, game, d, rnd);

  // set exit door position
  // bug: exit placed on generated entities. not so bad?
  Room& room = d.rooms[d.rooms.size() - 1];
  glm::ivec2 middle = room_center(room);
  create_dungeon_entity(editor, game, EntityType::tile_type_exit, middle);

  // Set the cost for all of the tiles
  for (const auto& tile : d.walls_and_floors) {
    const auto& entity = tile.entity;
    const auto& et = r.get<EntityTypeComponent>(entity);
    PathfindableComponent path;
    if (et.type == EntityType::tile_type_floor)
      path.cost = 0;
    else if (et.type == EntityType::tile_type_wall)
      path.cost = -1; // impassable
    else
      path.cost = 1;
    r.emplace<PathfindableComponent>(entity, path);
  }

  entt::entity e = r.create();
  r.emplace<EntityTypeComponent>(e, EntityType::empty);
  r.emplace<TagComponent>(e, "dungeon");
  r.emplace<Dungeon>(e, d);

  std::cout << "Dungeon generated!\n\n";
};

bool
transfer_old_state(GameEditor& editor, Game& game)
{
  const entt::registry old_r = std::move(game.state);
  bool did_transfer = false;
  entt::registry new_r;
  create_hierarchy_root_node(new_r);

  for (const auto [entity_player, old_player, old_xp, old_stats, old_type, old_hp] :
       old_r.view<PlayerComponent, XpComponent, StatsComponent, EntityTypeComponent, HealthComponent>().each()) {
    did_transfer = true;

    EntityType et = EntityType::actor_player;
    entt::entity e = create_gameplay(editor, new_r, et);
    create_renderable(editor, new_r, e, et);

    auto& xp = new_r.get<XpComponent>(e);
    xp.amount = old_xp.amount;

    auto& stats = new_r.get<StatsComponent>(e);
    stats.agi_level = old_stats.agi_level;
    stats.con_level = old_stats.con_level;
    stats.str_level = old_stats.str_level;
    stats.overall_level = old_stats.overall_level;

    auto& hp = new_r.get<HealthComponent>(e);
    hp.hp = old_hp.hp;

    // ... equipped
    for (const auto [entity_item, old_item, old_type] : old_r.view<IsEquipped, EntityTypeComponent>().each()) {
      if (old_item.parent != entity_player)
        continue; // not player equipment
      WantsToEquip& equip = new_r.get_or_emplace<WantsToEquip>(e);
      auto new_item = create_item(editor, new_r, old_type.type, e);
      equip.requests.push_back({ old_item.slot, new_item });
    }

    // ... in backpack
    for (const auto [entity_item, old_item, old_type] : old_r.view<InBackpackComponent, EntityTypeComponent>().each()) {
      if (old_item.parent != entity_player)
        continue; // not player equipment
      create_item(editor, new_r, old_type.type, e);
    }
  }

  game.state = std::move(new_r);
  return did_transfer;
}

} // namespace game2d

// reset game state. this is because I could not seem to
// correctly clear the registry AND the entt groups at the same time.
void
game2d::transfer_old_state_generate_dungeon(GameEditor& editor, Game& game, const int seed, const int floor)
{
  std::cout << "generating dungeon..." << std::endl;

  std::string msg = "Generating new dungeon! seed: " + std::to_string(seed) + ", floor: " + std::to_string(floor);
  game.ui_events.events.push_back(msg);

  bool transfered_player_data = transfer_old_state(editor, game);
  std::cout << "transferred: " << transfered_player_data << "\n";

  //
  // first time setup game setup
  //
  if (!transfered_player_data) {
    // players
    for (int i = 0; i < 1; i++) {
      EntityType et = EntityType::actor_player;
      entt::entity e = create_gameplay(editor, game, et);
      create_renderable(editor, game, e, et);

      // debug: give the player starting equipment
      // entt::entity sword_0 = create_item(editor, game.state, EntityType::sword, e);
      // entt::entity sword_1 = create_item(editor, game.state, EntityType::sword, e);
      // entt::entity shield_0 = create_item(editor, game.state, EntityType::shield, e);
      // entt::entity shield_1 = create_item(editor, game.state, EntityType::shield, e);
      // auto& purchase = game.state.get_or_emplace<WantsToPurchase>(e);
      // std::vector<entt::entity> items{
      //   sword_0, sword_1, shield_0, shield_1,
      // };
      // purchase.items.insert(purchase.items.end(), items.begin(), items.end());

      // entt::entity sword = create_item(editor, game.state, EntityType::sword, e);
      // entt::entity shield = create_item(editor, game.state, EntityType::shield, e);
      // WantsToEquip& equip = game.state.get_or_emplace<WantsToEquip>(e);
      // equip.requests.push_back({ EquipmentSlot::left_hand, sword });
      // equip.requests.push_back({ EquipmentSlot::right_hand, shield });
    }
  }
  init_input_system(game.input);

  int players = game.state.view<PlayerComponent>().size();
  std::cout << "players: " << players << "\n";

  int scale = static_cast<int>(20.0f * ((floor + 1) / 2.0f));
  generate_dungeon(editor, game, scale, scale, seed, floor);
  init_tile_fov_system(editor, game);
  create_gameplay(editor, game, EntityType::camera);

  // create_gameplay(editor, game, EntityType::actor_shopkeeper);
  // // stock up!
  // const auto& view = game.state.view<ShopKeeperComponent>();
  // view.each([&editor, &game](auto shop_entity, auto& shopkeeper) {
  //   create_item(editor, game, EntityType::potion, shop_entity);
  //   create_item(editor, game, EntityType::potion, shop_entity);
  //   create_item(editor, game, EntityType::potion, shop_entity);
  //   create_item(editor, game, EntityType::sword, shop_entity);
  //   create_item(editor, game, EntityType::shield, shop_entity);
  //   create_item(editor, game, EntityType::scroll_damage_nearest, shop_entity);
  //   // create_item(editor, game, EntityType::crossbow, shop_entity);
  //   // create_item(editor, game, EntityType::bolt, shop_entity);
  //   // create_item(editor, game, EntityType::scroll_damage_selected_on_grid, shop_entity);
  // });
};
