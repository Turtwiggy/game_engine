#include "dungeon_spawner_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/map/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/animations/rotate_components.hpp"
#include "modules/animations/wiggle/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/effects_outline/outline_components.hpp"
#include "modules/event_unit_enter_blackhole/unit_enter_blackhole_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/system_ai/system_ai_components.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"
#include "modules/ui_units/ui_units_components.hpp"

namespace game2d {

static auto seed = 0;
static auto enemy_rnd = engine::RandomState(seed);
static auto player_rnd = engine::RandomState(seed);
static auto rnd = engine::RandomState(seed);

void
add_initiative(entt::registry& r, entt::entity e)
{
#if defined(_DEBUG)
  static int init = 0;
  init++;
  r.emplace<InitiativeComponent>(e, init);
#else
  const int rnd_init = engine::rand_det_s(rnd.rng, 0, 20);
  r.emplace<InitiativeComponent>(e, rnd_init);
#endif
}

std::vector<entt::entity>
spawn_n_blackhole(entt::registry& r, std::vector<int>& idxs, int amount)
{
  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);

  std::vector<entt::entity> ents;

  int n_free_slots = static_cast<int>(idxs.size());
  do {
    // choose a random slot...
    const int slot_i = engine::rand_det_s(enemy_rnd.rng, 0, n_free_slots);
    const int slot_idx = idxs[slot_i];
    const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

    // impl
    const auto env_e = spawn_environment(r, "blackhole", pos);
    r.emplace<OnCollisionKill>(env_e);

    float rnd_angle = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI);
    set_dir(r, env_e, engine::angle_radians_to_direction(rnd_angle));
    float rnd_speed = engine::rand_det_s(rnd.rng, 0.5f, 1.2f);
    float rotate_speed = rnd_speed;
    r.emplace<AnimationRotate>(env_e, rotate_speed);
    r.emplace<SeparateTransformAndAABB>(env_e);

    add_entity_to_map(r, env_e, slot_idx);
    ents.push_back(env_e);

    // cleanup
    idxs.erase(idxs.begin() + slot_i); // remove slot from free slot
    amount--;
    n_free_slots--;

  } while (amount > 0 && n_free_slots > 0);

  return ents;
};

std::vector<entt::entity>
spawn_n_enemies(entt::registry& r, std::vector<int>& idxs, int amount)
{
  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);

  std::vector<entt::entity> ents;

  int n_free_slots = static_cast<int>(idxs.size());
  do {
    // choose a random slot...
    const int slot_i = engine::rand_det_s(enemy_rnd.rng, 0, n_free_slots);
    const int slot_idx = idxs[slot_i];
    const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

    {
      const auto mob_e = spawn_mob(r, "dungeon_actor_enemy_default");
      give_life(r, mob_e, pos);
      r.emplace<TeamComponent>(mob_e, TeamComponent{ AvailableTeams::enemy });
      r.emplace<DefaultBrainComponent>(mob_e);

      auto& inv = r.get<DefaultInventory>(mob_e).inv;
      auto& body = r.get<DefaultBody>(mob_e).body;

      // give the enemy a piece of scrap in their inventory
      spawn_inv_item(r, inv, 0, "scrap");

      // give the enemy a 5% chance to have a medkit in their inventory...
      // TODO: medkits

      // give enemy a weapon
      // todo: replace idx 6 with finding a slot the weapon should go
      auto weapon_e = spawn_inv_item(r, body, 0, "scrap_knife");

      add_initiative(r, mob_e);
      add_entity_to_map(r, mob_e, slot_idx);

      ents.push_back(mob_e);
    }

    idxs.erase(idxs.begin() + slot_i); // remove slot from free slot
    amount--;
    n_free_slots--;

  } while (amount > 0 && n_free_slots > 0);

  return ents;
};

std::vector<entt::entity>
spawn_n_players(entt::registry& r, std::vector<int>& idxs, const std::vector<UnitType>& units)
{
  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);

  std::vector<entt::entity> ents;

  int amount = units.size();
  int i = 0;
  int n_free_slots = static_cast<int>(idxs.size());
  do {
    // choose a random slot...
    const int slot_i = engine::rand_det_s(enemy_rnd.rng, 0, n_free_slots);
    const int slot_idx = idxs[slot_i];
    const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

    const auto unit_data = units[i];

    SDL_Log("Spawning player mob...");
    auto e = spawn_mob(r, "dungeon_actor_hero");
    give_life(r, e, pos);
    r.emplace<PlayerComponent>(e);
    r.emplace<TeamComponent>(e, AvailableTeams::player);
    r.emplace<InitBodyAndInventory>(e); // should be inventroy from active unit?
    r.emplace_or_replace<NameComponent>(e, NameComponent{ unit_data.name });
    r.emplace<UnitPersistentState>(e, UnitPersistentState{ unit_data.active, unit_data.permadead });
    r.emplace<SpriteOutline>(e);

    // WiggleUpAndDown wiggle_c;
    // wiggle_c.base_position = pos;
    // wiggle_c.amplitude = 1.0;
    // r.emplace<WiggleUpAndDown>(e, wiggle_c);

    add_initiative(r, e);
    add_entity_to_map(r, e, slot_idx);

    idxs.erase(idxs.begin() + slot_i); // remove slot from free slot
    amount--;
    n_free_slots--;

    i++;
    ents.push_back(e);
  } while (amount > 0 && n_free_slots > 0);

  return ents;
};

} // namespace game2d