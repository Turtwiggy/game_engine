#include "pch.hpp"

#include "engine/entt/helpers.hpp"
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
  }

  // level up the weapons
  for (const auto& weapon_e : evt.roll_result.weapons) {

    // add traits to all the weapons.
    auto& behaviours_c = r.get<WeaponBehaviourComponent>(weapon_e).behaviours;
    behaviours_c.insert(traits.begin(), traits.end());

    if (!evt.roll_result.level_weapons)
      continue; // dont level

    auto* wep_level_c = r.try_get<WeaponLevelComponent>(weapon_e);
    if (!wep_level_c)
      continue;

    wep_level_c->level++;

    auto it = std::find(core_weapon_levels.begin(), core_weapon_levels.end(), wep_level_c->level);
    if (it != core_weapon_levels.end()) {
      // if weapon level reaches a critical level,
      // send a weapon upgrade level event

      SDL_Log("Core weapon level reached: %i", wep_level_c->level);
      WeaponLevelReachedEvent lv_evt;
      lv_evt.level = wep_level_c->level;
      lv_evt.par_e = player_e;
      lv_evt.wep_e = weapon_e;
      evts_c.dispatcher->trigger(lv_evt);
      evts_c.dispatcher->update();
    }
  }

  //
}

} // namespace game2d