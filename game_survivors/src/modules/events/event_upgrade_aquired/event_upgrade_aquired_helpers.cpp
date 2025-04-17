#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "event_upgrade_aquired_helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/events/event_upgrade/event_upgrade_components.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"

namespace game2d {

// this probably shouldnt live here.
// std::vector<int> core_weapon_levels = { 5, 10, 15 };
const std::vector<int> core_weapon_levels = { 4, 8, 12 };

void
handle_upgrade_event(entt::registry& r, const UpgradeEvent& evt)
{
  auto& evts_c = get_first_component<SINGLE_Events>(r);

  auto player_e = evt.e;
  r.remove<UpgradeResultsComponent>(player_e); // done

  const auto rarity = evt.roll_result.rarity;
  const auto& stats = evt.roll_result.stats;
  const auto& traits = evt.roll_result.traits;
  const auto rarity_str = std::string(magic_enum::enum_name(evt.roll_result.rarity));

  // Upgrade stats.
  auto& stats_c = r.get<StatModifierComponent>(player_e);
  for (const auto& s : stats) {
    const auto& stat = s.stat;
    const auto& type = s.type;
    const auto& value = s.value;

    if (type == "stat_flat_increase")
      stats_c.add(std::make_shared<StatFlatIncrease>(value, stat));
    else if (type == "stat_percent_increase")
      stats_c.add(std::make_shared<StatPercentIncrease>(value, stat));
    else
      throw std::runtime_error("Unknown stat type");

    // If it's a weapon upgrade, upgrade the weapon level by 1.
    const auto stat_as_enum = magic_enum::enum_cast<UpgradeableStat>(stat).value();
    const auto& wab = weapon_and_bullet_stats;
    const bool is_wep_stat = std::find(wab.begin(), wab.end(), stat_as_enum) != wab.end();

    // note: added the level_weapon check here, become some weapon behaviours
    // e.g. HEAVY_PISTOL_CRIT contain stats that are WEAPON_X or BULLET_X,
    // but that itself is a level up-upgrade, so we dont want it to level up
    // due to the fact the weapons stats are being added
    //
    if (is_wep_stat && evt.roll_result.level_weapon) {
      auto& children_c = r.get<HasChildrenComponent>(player_e);
      for (const auto& child_e : children_c.children) {

        auto* wep_level_c = r.try_get<WeaponLevelComponent>(child_e);
        if (!wep_level_c)
          continue;

        // upgrade all weapons? this could be improved
        // by associating an upgrade with a weapon to level.
        wep_level_c->level++;

        auto it = std::find(core_weapon_levels.begin(), core_weapon_levels.end(), wep_level_c->level);
        if (it != core_weapon_levels.end()) {
          // if weapon level reaches a critical level,
          // send a weapon upgrade level event

          SDL_Log("Core weapon level reached: %i", wep_level_c->level);
          WeaponLevelReachedEvent lv_evt;
          lv_evt.level = wep_level_c->level;
          lv_evt.par_e = player_e;
          lv_evt.wep_e = child_e;
          evts_c.dispatcher->trigger(lv_evt);
          evts_c.dispatcher->update();
        }
      }
    }
  }

  // aquire a trait.
  for (const auto& trait : traits) {
    const auto trait_str = std::string(magic_enum::enum_name(trait));
    SDL_Log("player wants to aquire trait: %s", trait_str.c_str());
    auto& behaviours_c = r.get<WeaponBehaviourComponent>(player_e);
    behaviours_c.behaviours.emplace(trait);
  }
}

} // namespace game2d