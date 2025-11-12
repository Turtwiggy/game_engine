#include "pch.hpp"

#include "treasure_enemy_death_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_enemy_treasure/enemy_treasure_components.hpp"
#include "modules/systems/system_create_item/create_item_components.hpp"

namespace game2d {

enum class TreasureOption
{
  // GOLD = 0,
  HEALING_PACK = 0,
  SEA_MINE,
  VACUUM_ORB,

  count
};

constexpr std::array<std::pair<TreasureOption, int>, 5> treasure_chance_map = { {
  { TreasureOption::HEALING_PACK, 25 },
  { TreasureOption::SEA_MINE, 60 },
  { TreasureOption::VACUUM_ORB, 15 },
} };
constexpr int
sum_array_values()
{
  int sum = 0;
  for (const auto& pair : treasure_chance_map)
    sum += pair.second;
  return sum;
};
// check the probabilities are mathing to 100%
static_assert(sum_array_values() == 100);

void
handle_death_event__treasure_enemy(entt::registry& r, const DeathEvent& evt)
{
  auto dead_e = evt.dead;
  if (dead_e == entt::null)
    return;

  auto* treasure_enemy_c = r.try_get<TreasureEnemyComponent>(evt.dead);
  if (!treasure_enemy_c)
    return;

#if defined(_DEBUG)
  // static engine::RandomState angle_rnd(0); // same roll every time
  static engine::RandomState treasure_rnd(engine::get_system_time_for_seed());
#else
  static engine::RandomState treasure_rnd(engine::get_system_time_for_seed());
#endif

  // Spawn some treasure.
  int cumulative = 0;
  TreasureOption treasure = TreasureOption::SEA_MINE;
  const auto roll = engine::rand_det_s(treasure_rnd.rng, 0, 100);
  for (const auto& [t, val] : treasure_chance_map) {
    if (roll < cumulative) {
      treasure = t;
      break;
    }
    cumulative += val;
  }

  const auto treasure_enum = treasure;
  const auto treasure_str = std::string(magic_enum::enum_name(treasure_enum));
  SDL_Log("Treasure enemy died! Spawning treasure %s", treasure_str.c_str());

  entt::entity item_e = entt::null;

  // Dropping gold feels back to pick up
  // if (treasure_enum == TreasureOption::GOLD)
  //   create_empty<CreateItemRequest>(r, CreateItemRequest{ "item_gold", get_position(r, dead_e) });

  auto pos = get_position(r, dead_e);
  pos.x = floor(pos.x);
  pos.y = floor(pos.y);

  if (treasure_enum == TreasureOption::HEALING_PACK)
    create_empty<CreateItemRequest>(r, CreateItemRequest{ "item_hp_pack", pos });

  else if (treasure_enum == TreasureOption::SEA_MINE)
    create_empty<CreateItemRequest>(r, CreateItemRequest{ "item_sea_mine", pos });

  else if (treasure_enum == TreasureOption::VACUUM_ORB)
    create_empty<CreateItemRequest>(r, CreateItemRequest{ "item_vacuum_orb", pos });

  else
    throw std::runtime_error("Unknown item type");
}

} // namespace game2d