#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "event_weapon_level_reached_components.hpp"
#include "event_weapon_level_reached_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
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
        if (upgrade_key == upg_data.key && upg_data.game_key == behaviour_str) {
          // ... add the stats
          for (const auto& stat_data : upg_data.stats)
            stats.push_back(Stat{ .stat = stat_data.stat, .type = stat_data.type, .value = stat_data.value });
        }
      }
    }
  };

  return stats;
}

void
handle_weapon_level_reached_event(entt::registry& r, const WeaponLevelReachedEvent& evt)
{
  SDL_Log("weapon reached a level... generating upgrades.");

  const int num_players = 4;
  for (int i = 0; i < num_players; i++) {
    auto player_e = get_player_e_from_idx(r, i);
    if (player_e == entt::null)
      continue;
    const auto& player_c = r.get<PlayerComponent>(player_e);

    UpgradeResultsComponent results_c;

    // Generate 2 core weapon upgrades...
    // TODO: make random from selection of available upgrades for weapon.

    results_c.results.emplace(UpgradeRollResult{
      .rarity = Rarity::COMMON,
      .stats = get_stats_from_weapon_behaviour(r, WeaponBehaviour::SHOOT_BULLETS_OPPOSITE_DIRECTION),
      .traits = { WeaponBehaviour::SHOOT_BULLETS_OPPOSITE_DIRECTION },
      .level_weapon = false,
    });

    results_c.results.emplace(UpgradeRollResult{
      .rarity = Rarity::RARE,
      .stats = get_stats_from_weapon_behaviour(r, WeaponBehaviour::HEAVY_PISTOL_CRIT_UPGRADE),
      .traits = { WeaponBehaviour::HEAVY_PISTOL_CRIT_UPGRADE },
      .level_weapon = false,
    });

    r.emplace<UpgradeResultsComponent>(player_e, results_c);
  }

  auto& ui_c = get_first_component<SINGLE_LevelUpUI>(r);
  populate_ui_based_on_upgrades(r, ui_c);
}

} // namespace game2d