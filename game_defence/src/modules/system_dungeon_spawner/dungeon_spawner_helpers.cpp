#include "dungeon_spawner_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/map/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_unit_enter_blackhole/unit_enter_blackhole_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/system_ai/system_ai_components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"

namespace game2d {

static auto seed = 0;
static auto enemy_rnd = engine::RandomState(seed);
static auto player_rnd = engine::RandomState(seed);

void
spawn_n_blackhole(entt::registry& r, std::vector<int>& idxs, int amount)
{
  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);

  int n_free_slots = static_cast<int>(idxs.size());
  do {
    // choose a random slot...
    const int slot_i = engine::rand_det_s(enemy_rnd.rng, 0, n_free_slots);
    const int slot_idx = idxs[slot_i];
    const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

    // impl
    const auto env_e = spawn_environment(r, "blackhole", pos);
    r.emplace<OnCollisionKill>(env_e);

    add_entity_to_map(r, env_e, slot_idx);

    // cleanup
    idxs.erase(idxs.begin() + slot_i); // remove slot from free slot
    amount--;
    n_free_slots--;

  } while (amount > 0 && n_free_slots > 0);
};

void
spawn_n_enemies(entt::registry& r, std::vector<int>& idxs, int amount)
{
  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);

  int n_free_slots = static_cast<int>(idxs.size());
  do {
    // choose a random slot...
    const int slot_i = engine::rand_det_s(enemy_rnd.rng, 0, n_free_slots);
    const int slot_idx = idxs[slot_i];
    const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

    {
      const auto mob_e = spawn_mob(r, "dungeon_actor_enemy_default", pos);
      r.emplace<TeamComponent>(mob_e, TeamComponent{ AvailableTeams::enemy });
      r.emplace<DefaultBrainComponent>(mob_e);

      auto& inv = r.get<DefaultInventory>(mob_e).inv;
      auto& body = r.get<DefaultBody>(mob_e).body;

      // change the enemy difficulty
      r.get<HealthComponent>(mob_e).hp = 20;
      r.get<HealthComponent>(mob_e).max_hp = 20;

      // give the enemy a piece of scrap in their inventory
      spawn_inv_item(r, inv, 0, "scrap");

      // give the enemy a 5% chance to have a medkit in their inventory...
      // TODO: medkits

      // give enemy a weapon
      // todo: replace idx 6 with finding a slot the weapon should go
      auto weapon_e = spawn_inv_item(r, body, 0, "scrap_knife");

      add_entity_to_map(r, mob_e, slot_idx);
    }

    idxs.erase(idxs.begin() + slot_i); // remove slot from free slot
    amount--;
    n_free_slots--;

  } while (amount > 0 && n_free_slots > 0);
};

void
spawn_n_players(entt::registry& r, std::vector<int>& idxs, int amount)
{
  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);

  const int slot_idx = idxs[engine::rand_det_s(player_rnd.rng, 0, idxs.size())];
  const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

  auto e = spawn_mob(r, "dungeon_actor_hero", pos);
  // r.emplace<CircleComponent>(e);
  r.emplace<PlayerComponent>(e);
  r.emplace<TeamComponent>(e, AvailableTeams::player);
  r.emplace<InitBodyAndInventory>(e);
  // spawn_particle_emitter(r, "anything", pos, e);
  add_entity_to_map(r, e, slot_idx);
};

} // namespace game2d