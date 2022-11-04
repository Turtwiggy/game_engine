#include "helpers.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/dungeon/helpers/create.hpp"
#include "game/modules/dungeon/helpers/generate.hpp"
#include "game/modules/dungeon/helpers/rooms.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "game/modules/rpg_xp/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"

#include "magic_enum.hpp"

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
      d.occupied.push_back({ entity, grid_index });
    }
  }
}

void
generate_dungeon(GameEditor& editor, Game& game, const int size_x, const int size_y, uint32_t seed)
{
  const auto& colours = editor.colours;
  auto& r = game.state;

  Dungeon d;
  d.width = size_x;
  d.height = size_y;
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

  entt::entity e = r.create();
  r.emplace<EntityTypeComponent>(e, EntityType::empty);
  r.emplace<TagComponent>(e, "dungeon");
  r.emplace<Dungeon>(e, d);
};

// void
// transfer_old_state(GameEditor& editor, Game& game)
// {
//   // old gamestate of interest...
//   std::vector<HealthComponent> old_hp;
//   std::vector<XpComponent> old_xp;
//   std::vector<StatsComponent> old_stats;
//   std::vector<std::pair<IsEquipped, EntityType>> old_equipped_items;
//   std::vector<EntityType> old_items;
//   {
//     auto& old_r = game.state;
//     // ... player data
//     for (const auto [entity_player, player, xp, stats, type, hp] :
//          old_r.view<PlayerComponent, XpComponent, StatsComponent, EntityTypeComponent, HealthComponent>().each()) {
//       old_xp.push_back(xp);
//       old_stats.push_back(stats);
//       old_hp.push_back(hp);

//       // ... equipped items
//       for (const auto [item_eqp, item, type] : old_r.view<IsEquipped, EntityTypeComponent>().each()) {
//         if (item.parent != entity_player)
//           continue; // not player equipment
//         old_equipped_items.push_back({ item, type.type });
//       }

//       // ... items in backpack
//       for (const auto [item_ibp, item, type] : old_r.view<InBackpackComponent, EntityTypeComponent>().each()) {
//         if (item.parent != entity_player)
//           continue; // not a player item
//         old_items.push_back(type.type);
//       }
//     }
//   }

//   game.state = entt::registry();
//   auto& r = game.state;

//   // WARNING: below code is duplicate of game.cpp
//   // --------------------------------------------

//   create_hierarchy_root_node(r);

//   // players
//   for (int i = 0; i < 1; i++) {
//     EntityType et = EntityType::actor_player;
//     entt::entity e = create_gameplay(editor, game, et);
//     create_renderable(editor, r, e, et);
//   }

//   // ... copy over stats
//   for (int i = 0; const auto [entity, player, xp, stats, type, hp] :
//                   r.view<PlayerComponent, XpComponent, StatsComponent, EntityTypeComponent,
//                   HealthComponent>().each()) {
//     xp.amount = old_xp[i].amount;
//     stats.agi_level = old_stats[i].agi_level;
//     stats.con_level = old_stats[i].con_level;
//     stats.str_level = old_stats[i].str_level;
//     stats.overall_level = old_stats[i].overall_level;
//     hp.hp = old_hp[i].hp;

//     // ... give player items
//     std::cout << "items: " << old_items.size();
//     for (const auto& item : old_items)
//       create_item(editor, game, item, entity);

//     // ... equip the same items
//     WantsToEquip& equip = r.get_or_emplace<WantsToEquip>(entity);
//     for (const auto& [eitem, type] : old_equipped_items) {
//       auto new_item = create_item(editor, game, type, entity);
//       equip.requests.push_back({ eitem.slot, new_item });
//     }

//     break; // limit to one player for the moment
//     i++;
//   }

//   // camera
//   auto c = create_gameplay(editor, game, EntityType::camera);
//   r.emplace<TransformComponent>(c);
// };

} // namespace game2d

// reset game state. this is because I could not seem to
// correctly clear the registry AND use entt groups at the same time.
void
game2d::generate_dungeon_transfer_old_state(GameEditor& editor, Game& game, uint32_t& seed)
{
  // transfer_old_state(editor, game);

  seed += 1;

  std::string floor_msg = "Exit reached! New floor.";
  game.ui_events.events.push_back(floor_msg);

  std::string msg = "Generating new dungeon! seed: " + std::to_string(seed);
  game.ui_events.events.push_back(msg);

  // SET DUNGEON SIZE
  generate_dungeon(editor, game, 20, 20, seed);
}
