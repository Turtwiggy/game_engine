#include "treasure_enemy_death_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "magic_enum.hpp"
#include "modules/actor_enemy_treasure/enemy_treasure_components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/event_coll_player_hp/event_coll_player_hp_components.hpp"
#include "modules/event_coll_player_sea_mine/event_coll_player_sea_mine_components.hpp"
#include "modules/event_coll_player_vacuum_orb/event_coll_player_vacuum_orb_components.hpp"
#include "modules/system_create_item/create_item_components.hpp"

#include <SDL2/SDL_log.h>
#include <stdexcept>

namespace game2d {

enum class TreasureOption
{
  GOLD = 0,
  HEALING_PACK,
  SEA_MINE,
  VACUUM_ORB,

  count
};

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
  const auto treasure_idx = engine::rand_det_s(treasure_rnd.rng, 0, (int)TreasureOption::count);
  const auto treasure_enum = magic_enum::enum_value<TreasureOption>(treasure_idx);
  const auto treasure_str = std::string(magic_enum::enum_name(treasure_enum));
  SDL_Log("Treasure enemy died! Spawning treasure %s", treasure_str.c_str());

  entt::entity item_e = entt::null;

  if (treasure_enum == TreasureOption::GOLD)
    create_empty<CreateItemRequest>(r, CreateItemRequest{ "item_gold", get_position(r, dead_e) });

  else if (treasure_enum == TreasureOption::HEALING_PACK)
    create_empty<CreateItemRequest>(r, CreateItemRequest{ "item_hp_pack", get_position(r, dead_e) });

  else if (treasure_enum == TreasureOption::SEA_MINE)
    create_empty<CreateItemRequest>(r, CreateItemRequest{ "item_sea_mine", get_position(r, dead_e) });

  else if (treasure_enum == TreasureOption::VACUUM_ORB)
    create_empty<CreateItemRequest>(r, CreateItemRequest{ "item_vacuum_orb", get_position(r, dead_e) });

  else
    throw std::runtime_error("Unknown item type");
}

} // namespace game2d