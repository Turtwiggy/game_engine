#include "modules/actors/actor_player/components.hpp"
#include "pch.hpp"

#include "event_shoot_muzzleflash.hpp"

namespace game2d {

void
handle_shoot_event__muzzleflash(entt::registry& r, const ShootEvent& evt)
{
  const auto par_e = evt.parent_e;
  const auto wep_e = evt.weapon_e;

  if (!r.all_of<PlayerComponent>(par_e))
    return;

  // todo: spawn a muzzle vfx
}

} // namespace game2d