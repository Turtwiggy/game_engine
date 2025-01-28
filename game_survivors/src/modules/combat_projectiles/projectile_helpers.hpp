#pragma once

#include "modules/combat/components.hpp"
#include "modules/system_traits/trait_components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct BulletDef
{
  std::string key = "bullet_default";
  entt::entity parent_e = entt::null;
  glm::vec2 size{ 10, 10 };
  AvailableTeams team = AvailableTeams::neutral;
  int damage = 0;
  int pierce = 1;
  int speed = 250;
  int lifecycle = 3 * 1000;
  int knockback_force = 50;
  std::vector<AquirableTrait> traits;
};

entt::entity
spawn_projectile(entt::registry& r, const BulletDef& bullet_def);

//

} // namespace game2d