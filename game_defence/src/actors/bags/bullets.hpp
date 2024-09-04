#pragma once

#include "core.hpp"

#include "modules/combat_damage/components.hpp"

namespace game2d {

enum class BulletType
{
  DEFAULT,
  BOUNCY,
};

struct BulletData
{
  BulletType type = BulletType::DEFAULT;

  float bullet_damage = 12.0f;
  float bullet_speed = 250.0f;
  float rotation = 0.0f;
  glm::vec2 dir{ 0.0f };
  AvailableTeams team = AvailableTeams::neutral;
  entt::entity parent = entt::null;
};

struct DataBullet
  : public EntityData
  , public BulletData
{

  DataBullet() = delete;

  DataBullet(const BulletType& in_type)
  {
    type = in_type;

    switch (in_type) {
      case BulletType::BOUNCY: {
        icon = "AMMO_BOX";
        sprite = "EMPTY";
        break;
      }
      case BulletType::DEFAULT: {
        icon = "AMMO_BOX";
        sprite = "EMPTY";
        break;
      }
    }

    //
  };
};

} // namespace game2d