#include "weapons.hpp"

namespace game2d {

MeleeWeaponStats
create_shovel()
{
  MeleeWeaponStats s;
  s.damage = 1;
  return s;
};

RangedWeaponStats
create_pistol()
{
  RangedWeaponStats r;
  r.damage = 3;
  r.radius_offset_from_player = 14;
  r.infinite_ammo = true;
  r.current_ammo = 0;
  r.fire_rate_seconds_limit = 0.2f;
  return r;
};

RangedWeaponStats
create_shotgun()
{
  RangedWeaponStats r;
  r.damage = 5;
  r.radius_offset_from_player = 17;
  r.infinite_ammo = false;
  r.current_ammo = 0;
  r.fire_rate_seconds_limit = 0.5f;
  return r;
};

RangedWeaponStats
create_machinegun()
{
  RangedWeaponStats r;
  r.damage = 4;
  r.radius_offset_from_player = 16;
  r.infinite_ammo = false;
  r.current_ammo = 0;
  r.fire_rate_seconds_limit = 0.3f;
  return r;
};

}