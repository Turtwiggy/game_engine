#include "pch.hpp"

#include "event_shoot_turrets_helpers.hpp"

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
#include "modules/systems/system_weapon_sea_turret/weapon_sea_turret_components.hpp"

namespace game2d {

void
spawn_sea_turret(entt::registry& r, entt::entity wep_e, entt::entity parent_e)
{
  // deploy a thing!

  const auto& weapons = get_first_component<SINGLE_Weapons>(r);

  // treat turrets as heavy pistols?
  // Note: upgrades apply to both the turret weapon (i.e. turret launcher,)
  // and the weapon that the sea-turret is spawned with (e.g. heavy pistol)
  // const auto sea_turret_wep_data = r.get<const Weapon_OnDiskData>(wep_e);

  const auto heavy_pistol_data = weapons.weapons[0]; // todo: dont use idx
  auto turret_e = spawn_weapon(r, wep_e, heavy_pistol_data, "weapon_sea_turret");
  set_position(r, turret_e, get_position(r, wep_e));
  set_colour(r, turret_e, r.get<DefaultColour>(parent_e).colour);

  // turret-specific components
  r.emplace<EntityTimedLifecycle>(turret_e, 6 * 1000);
  r.emplace<AutofireComponent>(turret_e);
  r.emplace<HasParentComponent>(turret_e, wep_e);
}

void
handle_shoot_event__deploy_turrets(entt::registry& r, const ShootEvent& evt)
{
  const auto from_e = evt.parent_e;
  const auto wep_e = evt.weapon_e;

  if (from_e == entt::null || wep_e == entt::null)
    return;

  if (!r.all_of<WeaponSeaTurret>(wep_e))
    return;

  spawn_sea_turret(r, wep_e, from_e);
}

} // namespace game2d