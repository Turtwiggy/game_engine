#pragma once

#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

struct WeaponBehaviourMegabulletIn
{
  WeaponDef wep_def;
  BulletDef bul_def;
};
struct WeaponBehaviourMegabulletOut
{
  WeaponDef wep_def;
  BulletDef bul_def;
};
WeaponBehaviourMegabulletOut
weapon_behaviour_megabullet(entt::registry& r, const WeaponBehaviourMegabulletIn& in);

//
//

struct WeaponBehaviourBulletOppositeDirectionIn
{
  entt::entity wep_e;
  WeaponDef wep_def;
  BulletDef bul_def;
  glm::vec2 wep_pos;
  std::vector<float> angles_rad;
};
struct WeaponBehaviourBulletOppositeDirectionOut
{
  bool placeholder = true;
};
WeaponBehaviourBulletOppositeDirectionOut
weapon_behaviour_shoot_in_opposite_direction(entt::registry& r, const WeaponBehaviourBulletOppositeDirectionIn& in);

} // namespace game2d