#include "units.hpp"

#include "actors/actors.hpp"
#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_drop_items/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"

namespace game2d {

using FreeSlots = std::vector<int>;
static engine::RandomState rnd(0);

void
spawn_enemy(entt::registry& r, const int idx)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  DataDungeonActor desc;
  desc.pos = engine::grid::index_to_world_position_center(idx, map_c.xmax, map_c.ymax, map_c.tilesize);
  desc.team = AvailableTeams::enemy;
  desc.hp = 50;
  desc.max_hp = 50;
  desc.colour = engine::SRGBColour{ 0.7f, 0.3f, 0.3f, 1.0f };
  const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);

  // give the enemy a piece of scrap in their inventory
  create_inv_scrap(r, r.get<DefaultInventory>(dungeon_e).inv[0]);

  // give the enemy a 5% chance to have a medkit in their inventory...
  // TODO: medkits

  // get enemies to their inventory
  r.emplace<DropItemsOnDeathComponent>(dungeon_e);

  // give enemies a shotgun
  DataWeaponShotgun wdesc;
  wdesc.able_to_shoot = true;
  wdesc.parent = dungeon_e;
  wdesc.team = desc.team;
  const auto weapon_e = Factory_DataWeaponShotgun::create(r, wdesc);
};

void
spawn_cover(entt::registry& r, const int idx)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  DataDungeonCover desc;
  desc.pos = engine::grid::index_to_world_position_center(idx, map_c.xmax, map_c.ymax, map_c.tilesize);
  // fmt::println("cover generated: {},{}", desc.pos.x, desc.pos.y);

  const auto e = Factory_DataDungeonCover::create(r, desc);
};

FreeSlots
get_free_slots_idxs(const MapComponent& map_c, const Room& room)
{
  FreeSlots results;

  const glm::ivec2 tl = room.tl;
  const glm::ivec2 br = room.tl + glm::ivec2{ room.aabb.size.x, room.aabb.size.y };

  for (int x = tl.x + 1; x < br.x - 1; x++) {
    for (int y = tl.y + 1; y < br.y - 1; y++) {
      const auto idx = engine::grid::grid_position_to_index({ x, y }, map_c.xmax);

      // TODO: investigate this. stuff now contains loot on the floor,
      // which means that this cell WOULD technically be "free"

      if (map_c.map[idx].size() != 0)
        continue; // not free...

      results.push_back(idx);
    }
  }

  return results;
};

void
spawn_enemy_in_free_slots(entt::registry& r, FreeSlots& free_slots, int amount)
{
  for (int i = amount; i > 0; i--) {
    const int n_free_slots = static_cast<int>(free_slots.size());
    if (n_free_slots == 0)
      return;

    // choose a random free slot
    const int slot_i = engine::rand_det_s(rnd.rng, 0, n_free_slots);
    const int slot_idx = free_slots[slot_i];

    spawn_enemy(r, slot_idx);

    // remove slot from free slot
    free_slots.erase(free_slots.begin() + slot_i);
  }
};

void
spawn_cover_in_free_slots(entt::registry& r, FreeSlots& free_slots, int amount)
{
  for (int i = amount; i > 0; i--) {
    const int n_free_slots = static_cast<int>(free_slots.size());
    if (n_free_slots == 0)
      return;

    // choose a random free slot
    const int slot_i = engine::rand_det_s(rnd.rng, 0, n_free_slots);
    const int slot_idx = free_slots[slot_i];

    spawn_cover(r, slot_idx);

    // remove slot from free slot
    free_slots.erase(free_slots.begin() + slot_i);
  }
};

void
set_generated_entity_positions(entt::registry& r, DungeonGenerationResults& results, engine::RandomState& rnd)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  std::vector<std::pair<Room, FreeSlots>> room_to_free_slots;

  // i = 0 is the player room
  for (size_t i = 1; i < results.rooms.size(); i++) {
    const Room& room = results.rooms[i];
    const auto free_slots = get_free_slots_idxs(map_c, room);
    room_to_free_slots.push_back({ room, free_slots });
  }

  // Generate some things
  for (auto& [room, free_slots] : room_to_free_slots) {
    spawn_enemy_in_free_slots(r, free_slots, 3);
    spawn_cover_in_free_slots(r, free_slots, 1);
  }

  //
};

} // namespace game2d