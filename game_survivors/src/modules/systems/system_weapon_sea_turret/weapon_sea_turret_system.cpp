#include "pch.hpp"

#include "weapon_sea_turret_components.hpp"
#include "weapon_sea_turret_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/events/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/events/event_shoot/event_shoot_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_autofire/autofire_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"

namespace game2d {

void
spawn_sea_turret(entt::registry& r, entt::entity wep_e, entt::entity parent_e)
{
  // deploy a thing!

  const auto& weapons = get_first_component<SINGLE_Weapons>(r);

  // treat turrets as heavy pistols?
  // Note: upgrades apply to both the turret weapon (i.e. turret launcher,)
  // and the weapon that the sea-turret is spawned with (e.g. heavy pistol)

  const auto heavy_pistol_data = weapons.weapons[0];                // todo: dont use idx
  const auto sea_turret_wep_data = r.get<Weapon_OnDiskData>(wep_e); // alt: get from weapons.weapons

  auto turret_e = spawn_weapon(r, heavy_pistol_data, "weapon_sea_turret");
  set_position(r, turret_e, get_position(r, wep_e));
  set_colour(r, turret_e, r.get<DefaultColour>(parent_e).colour);

  // turret-specific components
  r.emplace<EntityTimedLifecycle>(turret_e, 6 * 1000);
  r.emplace<AutofireComponent>(turret_e);

  // NOTE: sea-turret parent is not weapon but weapon's parent
  // This means that upgrades filter through to the weapon
  r.emplace<HasParentComponent>(turret_e, HasParentComponent{ parent_e });
}

void
update_weapon_sea_turret_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  for (const auto& [wep_e, turret_c, weapon_fire_rate_c, parent_c, weapon_c, weapon_reload_rate_c, weapon_clip_size_c] :
       r.view<WeaponSeaTurret, WeaponFireRate, HasParentComponent, Weapon_OnDiskData, WeaponReloadRate, WeaponClipSize>()
         .each()) {

    if (weapon_reload_rate_c.seconds_cur > 0.0) {
      weapon_reload_rate_c.seconds_cur -= dt;
      continue;
    }

    const auto p = parent_c.parent;
    const auto wep_def = get_weapon_def(r, p, wep_e);
    const auto bul_def = get_bullet_def(r, p, wep_e);

    // you've reloaded
    if (weapon_clip_size_c.bullets_cur <= 0) {
      weapon_clip_size_c.bullets_cur = wep_def.bullets_max;
      weapon_fire_rate_c.seconds_between_shots_left = 0.0f;
    }

    // Check if you're fire-rate limited.
    // note: updates the _max time based on the modded firerate
    weapon_fire_rate_c.seconds_between_shots_max = 1.0f / wep_def.fire_rate;
    if (weapon_fire_rate_c.seconds_between_shots_left >= 0.0) {
      weapon_fire_rate_c.seconds_between_shots_left -= dt;
      continue;
    }

    // Check the clip size before firing.
    if (weapon_clip_size_c.bullets_cur <= 0) { // time to reload
      weapon_reload_rate_c.seconds_cur = wep_def.reload_rate;
      continue;
    }

    // Shoot a bullet! (in this case, the bullet is a turret)
    weapon_clip_size_c.bullets_cur--;
    weapon_fire_rate_c.seconds_between_shots_left = weapon_fire_rate_c.seconds_between_shots_max;

    SDL_Log("Spawning sea turret...");
    spawn_sea_turret(r, wep_e, p);

    // Check the clip size after firing.
    if (weapon_clip_size_c.bullets_cur <= 0)
      weapon_reload_rate_c.seconds_cur = wep_def.reload_rate;

    ShootEvent shoot_evt;
    shoot_evt.parent_e = p;
    shoot_evt.weapon_e = wep_e;
    evts_c.dispatcher->trigger(shoot_evt);
    evts_c.dispatcher->update();
  }
}

} // namespace game2d