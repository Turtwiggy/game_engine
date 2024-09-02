#pragma once

#include "core.hpp"
#include "modules/combat_damage/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

struct DataWeaponShotgun : public EntityData
{
  entt::entity parent = entt::null;
  AvailableTeams team = AvailableTeams::neutral;
  bool able_to_shoot = false;

  DataWeaponShotgun()
  {
    icon = "WEAPON_SHOTGUN";
    sprite = "WEAPON_SHOTGUN";
  }
};

} // namespace game2d
