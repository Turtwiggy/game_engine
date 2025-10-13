#include "pch.hpp"

#include "event_shoot_island_turret.hpp"

#include "modules/actors/actor_island_cannon/island_cannon_components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"

namespace game2d {

void
handle_shoot_event__island_turret(entt::registry& r, const ShootEvent& evt)
{
  const auto wep_e = evt.weapon_e;

  auto* cannon_c = r.try_get<IslandCannonComponent>(wep_e);
  if (!cannon_c)
    return;

  auto& mag_c = r.get<WeaponClipSize>(wep_e);
  // mag_c->bullets_left--;
  // mag_c->bullets_left = glm::max(cannon_c->mag_c, 0);

  // if you're out of bullets, require repairing again.
  if (mag_c.bullets_cur <= 0) {
    SDL_Log("Cannon out of ammo... setting to broken.");
    cannon_c->state = IslandCannonState::BROKEN;
    cannon_c->hits_to_repair_left = cannon_c->hits_to_repair;
  }
}

} // namespace game2d