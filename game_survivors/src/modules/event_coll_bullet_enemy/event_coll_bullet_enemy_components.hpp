#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct BulletComponent
{
  bool placeholder = true;
};

struct BulletDamage
{
  int damage = 10;
};

struct BulletPierce
{
  int pierce = 1;
  int pierced = 0;
};

} // namespace game2d