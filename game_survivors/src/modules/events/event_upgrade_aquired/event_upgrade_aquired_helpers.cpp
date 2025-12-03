#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "event_upgrade_aquired_helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/events/event_upgrade/event_upgrade_components.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_particles/components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"

namespace game2d {

// this probably shouldnt live here.
// std::vector<int> core_weapon_levels = { 5, 10, 15 };
const std::vector<int> core_weapon_levels = { 4, 8, 12 };

void
handle_upgrade_event(entt::registry& r, const UpgradeEvent& evt)
{
  auto& evts_c = SINGLE_Events::instance;

  // note: upg_e is either on the player, or on the weapon
  const auto par_e = evt.par_e;
  const auto upg_e = evt.upg_e;

  // done
  remove_if_exists<UpgradeResultsComponent>(r, par_e);
  remove_if_exists<UpgradeResultsComponent>(r, upg_e);

  const auto rarity = evt.roll_result.rarity;
  const auto& stats = evt.roll_result.stats;
  const auto& traits = evt.roll_result.traits;
  const auto rarity_str = std::string(magic_enum::enum_name(evt.roll_result.rarity));

  {
    RequestToSpawnParticles request;
    request.particle_type = ParticleType::VFX_LEVELUP_INNER;
    request.position = get_position(r, par_e);
    create_empty<RequestToSpawnParticles>(r, request);
  }
  {
    RequestToSpawnParticles request;
    request.particle_type = ParticleType::VFX_LEVELUP_OUTER;
    request.position = get_position(r, par_e);
    create_empty<RequestToSpawnParticles>(r, request);
  }

  // Upgrade stats
  auto& stats_c = r.get<StatModifierComponent>(upg_e);
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

  // Upgrade weapons sent in request.
  for (const auto weapon_e : evt.roll_result.weapons) {

    // add traits to all the weapons.
    auto& behaviours_c = r.get<WeaponBehaviourComponent>(weapon_e).behaviours;
    behaviours_c.insert(traits.begin(), traits.end());

    if (!evt.roll_result.level_weapons)
      continue; // dont level

    auto& wep_level_c = r.get<WeaponLevelComponent>(weapon_e);
    wep_level_c.level++;

    auto it = std::find(core_weapon_levels.begin(), core_weapon_levels.end(), wep_level_c.level);
    if (it == core_weapon_levels.end())
      continue; // not a core weapon level

    SDL_Log("Core weapon level reached: %i", wep_level_c.level);
    WeaponLevelReachedEvent lv_evt;
    lv_evt.level = wep_level_c.level;
    lv_evt.par_e = par_e;
    lv_evt.wep_e = weapon_e;
    evts_c.dispatcher->trigger(lv_evt);
    evts_c.dispatcher->update();
  }

  // Update WeaponDef and BulletDef
  if (r.all_of<WeaponComponent>(upg_e)) {
    SDL_Log("Updating WeaponDef & BulletDef");
    r.emplace_or_replace<WeaponDef>(upg_e, get_weapon_def(r, upg_e));

    if (r.all_of<BulletDef>(upg_e))
      r.emplace_or_replace<BulletDef>(upg_e, get_bullet_def(r, upg_e));
  } else
    SDL_Log("The thing that was upgraded wasnt a weapon (maybe an actor e.g. boat)");

  //
}

} // namespace game2d