#pragma once

#include <entt/entt.hpp>

namespace game2d {

enum class ENTITY_TYPE
{
  ENEMY,
  PLAYER,
  SWORD,
  FIRE_SWORD,
  CROSSBOW,
  BULLET,
  SHIELD,
  POTION,

  COUNT
};

entt::entity
create_item(entt::registry& r, const ENTITY_TYPE& type, const entt::entity& parent);

entt::entity
create_entity(entt::registry&, const ENTITY_TYPE& type);

} // namespace game2d
