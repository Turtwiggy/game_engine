#pragma once

#include "modules/combat/components.hpp"
#include "modules/system_traits/trait_components.hpp"

#include <SDL2/SDL_log.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <stdexcept>

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
  int bounces = 0;
  std::vector<AquirableTrait> traits;

  BulletDef() = delete;
  BulletDef(entt::entity parent)
    : parent_e(parent)
  {
    if (parent == entt::null) {
      throw std::runtime_error("Invalid bullet parent");
      exit(1); // crash app
    }
  };
};

struct WeaponDef
{
  float firerate = 0.5f;
  int projectiles = 1;
  int spread_deg = 30;
};

} // namespace game2d