#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "event_weapon_level_reached_components.hpp"
#include "event_weapon_level_reached_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"

namespace game2d {

std::vector<Stat>
get_stats_from_weapon_behaviour(entt::registry& r, const WeaponBehaviour behaviour)
{
  const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
  const auto behaviour_str = std::string(magic_enum::enum_name(behaviour));

  // search SINGLE_weapons for wep_behaviour stats
  std::vector<Stat> stats;
  for (const Weapon_OnDiskData& data : weapons_c.weapons) {
    for (const std::string& upgrade_key : data.upgrades) {
      for (const WeaponUpgrade_OnDiskData& upg_data : weapons_c.weapon_upgrades) {
        // If the weapon has a weapon-upgrade, and we want those stats...
        if (upgrade_key == upg_data.u_key && upg_data.wb_key == behaviour_str) {
          // ... add the stats
          for (const auto& stat_data : upg_data.stats)
            stats.push_back(Stat{ .stat = stat_data.stat, .type = stat_data.type, .value = stat_data.value });
        }
      }
    }
  };

  return stats;
}

std::vector<std::string>
get_weapon_upgrade_keys(entt::registry& r, const std::string weapon_key)
{
  const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);

  for (const auto& weapon : weapons_c.weapons)
    if (weapon.key == weapon_key)
      return weapon.upgrades;

  return {};
}

std::vector<WeaponUpgrade_OnDiskData>
get_upgrades_from_weapon_key(entt::registry& r, const std::string weapon_key)
{
  const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
  const auto u_keys = get_weapon_upgrade_keys(r, weapon_key);

  std::vector<WeaponUpgrade_OnDiskData> results;

  for (const auto& wep_upg : weapons_c.weapon_upgrades) {
    const auto u_key = wep_upg.u_key;
    auto it = std::find(u_keys.begin(), u_keys.end(), u_key);
    if (it != u_keys.end())
      results.push_back(wep_upg);
  }

  return results;
}

std::vector<std::string>
get_aquired_upgrades(entt::registry& r,
                     const std::vector<WeaponUpgrade_OnDiskData>& weapon_upgrades_data,
                     entt::entity player_e)
{
  std::vector<std::string> upgrades;

  // convert the weapon behaviours to a list of aquired upgrade keys
  const auto& aquired_wb = r.get<WeaponBehaviourComponent>(player_e).behaviours;

  for (const auto& weapon_upgrade : weapon_upgrades_data) {
    auto wb_enum = magic_enum::enum_cast<WeaponBehaviour>(weapon_upgrade.wb_key).value();
    auto it = std::find(aquired_wb.begin(), aquired_wb.end(), wb_enum);
    if (it != aquired_wb.end())
      upgrades.push_back(weapon_upgrade.u_key);
  }

  return upgrades;
}

std::string
get_wb_key_from_upgrade_key(entt::registry& r, const std::string& u_key)
{
  const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
  for (const auto& upgr : weapons_c.weapon_upgrades) {
    if (upgr.u_key == u_key)
      return upgr.wb_key;
  }
  const auto err = std::format("unable to find weapon upgrade key: {}", u_key);
  throw std::runtime_error(err);
  return "";
}

void
handle_weapon_level_reached_event(entt::registry& r, const WeaponLevelReachedEvent& evt)
{
  const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);

  SDL_Log("weapon reached a level... generating upgrades.");

  const int num_players = 4;
  for (int i = 0; i < num_players; i++) {
    auto player_e = get_player_e_from_idx(r, i);
    if (player_e == entt::null)
      continue;
    const auto& player_c = r.get<PlayerComponent>(player_e);

    UpgradeResultsComponent results_c;

    // Get the player's weapon
    entt::entity weapon_e = entt::null;
    const auto& player_children_c = r.get<HasChildrenComponent>(player_e);
    for (const auto& child_e : player_children_c.children) {
      if (auto* weapon_c = r.try_get<WeaponComponent>(child_e)) {
        weapon_e = child_e;
        break; // only consider first weapon, for the moment.
      }
    }

    if (weapon_e == entt::null) {
      SDL_Log("Player has no weapon equipped?");
      continue;
    }

    // Generate 2 (unowned) core weapon upgrades...
    const int upgrades = 2;

    const auto& weapon_key = r.get<ItemKey>(weapon_e);
    const auto weapon_upgrades_data = get_upgrades_from_weapon_key(r, weapon_key.key);

    std::vector<std::string> upgs;
    for (const auto& data : weapon_upgrades_data)
      upgs.push_back(data.u_key);

    // aquired upgrades
    const auto aquired_upg = get_aquired_upgrades(r, weapon_upgrades_data, player_e);

    // unaquired upgrades
    std::vector<std::string> unaquired_upg;
    for (const auto& upg : upgs) {
      auto it = std::find(aquired_upg.begin(), aquired_upg.end(), upg);
      if (it == aquired_upg.end())
        unaquired_upg.push_back(upg);
    }

    if (unaquired_upg.size() < upgrades) {
      auto msg = "Available upgrades <2! Weapons should have >5 upgrades, and you aquire 3";
      throw std::runtime_error(msg);
    }

#if defined(_DEBUG)
    // static engine::RandomState roll_rnd(0); // same roll every time
    static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#else
    static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#endif

    for (int i = 0; i < upgrades; i++) {
      const auto chosen_i = engine::rand_det_s(roll_rnd.rng, 0, (int)unaquired_upg.size());
      const auto upgrade_key = unaquired_upg[chosen_i];
      const auto wb_key = get_wb_key_from_upgrade_key(r, upgrade_key);
      const auto behaviour = magic_enum::enum_cast<WeaponBehaviour>(wb_key).value();

      results_c.results.emplace(UpgradeRollResult{
        .rarity = Rarity::COMMON,
        .stats = get_stats_from_weapon_behaviour(r, behaviour),
        .traits = { behaviour },
        .level_weapon = false,
      });

      std::erase(unaquired_upg, upgrade_key);
    }

    r.emplace<UpgradeResultsComponent>(player_e, results_c);
  }

  auto& ui_c = get_first_component<SINGLE_LevelUpUI>(r);
  populate_ui_based_on_upgrades(r, ui_c);
}

} // namespace game2d