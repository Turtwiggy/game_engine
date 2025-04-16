#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "event_upgrade_aquired_helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/events/event_upgrade/event_upgrade_components.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_components.hpp"
#include "modules/events/events_core/events_components.hpp"

namespace game2d {

// this probably shouldnt live here.
// std::vector<int> core_weapon_levels = { 5, 10, 15 };
std::vector<int> core_weapon_levels = { 2, 3, 4 };

void
handle_upgrade_event(entt::registry& r, const UpgradeEvent& evt)
{
  auto& evts_c = get_first_component<SINGLE_Events>(r);

  auto player_e = evt.e;

  // upgrade a stat.
  if (evt.roll_result.value.stat.has_value()) {
    auto type_str = evt.type;
    const auto upgrade = evt.roll_result.value.stat.value();
    const auto rarity_str = std::string(magic_enum::enum_name(evt.roll_result.rarity));
    const auto upgrade_str = std::string(magic_enum::enum_name(upgrade));
    const auto amount = evt.value;

    auto& stats_c = r.get<StatModifierComponent>(player_e);
    if (type_str == "stat_flat_increase")
      stats_c.add(std::make_shared<StatFlatIncrease>(amount, upgrade_str));
    else if (type_str == "stat_percent_increase")
      stats_c.add(std::make_shared<StatPercentIncrease>(amount, upgrade_str));
    else
      throw std::runtime_error("Unknown stat type");
  }

  r.remove<UpgradeResultsComponent>(player_e); // done

  // If it's a weapon upgrade, upgrade the weapon level by 1.
  if (evt.roll_result.value.stat.has_value()) {
    const auto upgrade = evt.roll_result.value.stat.value();
    const auto& wab = weapon_and_bullet_stats;
    const bool is_wep_stat = std::find(wab.begin(), wab.end(), upgrade) != wab.end();
    if (is_wep_stat) {
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
  if (evt.roll_result.value.trait.has_value()) {
    const auto trait = evt.roll_result.value.trait.value();
    const auto trait_str = std::string(magic_enum::enum_name(trait));
    SDL_Log("player wants to aquire trait: %s", trait_str.c_str());

    auto& behaviours_c = r.get<WeaponBehaviourComponent>(player_e);
    behaviours_c.traits.emplace(trait);
  }
}

} // namespace game2d