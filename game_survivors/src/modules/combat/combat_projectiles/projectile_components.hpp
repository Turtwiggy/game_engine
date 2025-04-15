#pragma once

#include "modules/combat/combat_core/components.hpp"
#include "modules/systems/system_traits/trait_components.hpp"

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct BulletDef
{
  std::string key = "bullet_default";
  entt::entity parent_e = entt::null;
  glm::vec2 size{ 10, 10 };
  AvailableTeams team = AvailableTeams::neutral;
  int damage = 0;
  int pierce = 1;
  float speed = 1.0f; // m/s
  int lifecycle = 3 * 1000;
  float knockback_force = 1.0f;
  int bounces = 0;
  float crit_chance = 0.0f;   // percent
  float crit_damage = 100.0f; // percent of your base damage
  float lifesteal = 0;
  std::vector<AquirableTrait> traits;

  BulletDef() = delete;
  BulletDef(entt::entity parent)
    : parent_e(parent) {
      // if (parent == entt::null) {
      //   throw std::runtime_error("Invalid bullet parent");
      //   exit(1); // crash app
      // }
    };
};

struct WeaponDef
{
  int projectiles = 1;
  int spread_deg = 30;
  float fire_rate = 0.5f;
  float reload_rate = 0;
  float range = 0;
  int bullets_max = 0;
};

} // namespace game2d