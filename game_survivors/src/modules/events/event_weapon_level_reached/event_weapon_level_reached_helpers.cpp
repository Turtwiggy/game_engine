#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "event_weapon_level_reached_components.hpp"
#include "event_weapon_level_reached_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"

namespace game2d {

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
      .value = UpgradeValue{ .trait = WeaponBehaviour::SHOOT_BULLETS_OPPOSITE_DIRECTION },
    });

    results_c.results.emplace(UpgradeRollResult{
      .rarity = Rarity::RARE,
      .value = UpgradeValue{ .trait = WeaponBehaviour::HEAVY_PISTOL_CRIT_UPGRADE },
    });

    r.emplace<UpgradeResultsComponent>(player_e, results_c);
  }

  auto& ui_c = get_first_component<SINGLE_LevelUpUI>(r);
  populate_ui_based_on_upgrades(r, ui_c);
}

} // namespace game2d