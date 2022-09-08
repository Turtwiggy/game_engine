#pragma once

#include "engine/colour.hpp"

#include <entt/entt.hpp>

#include <optional>

namespace game2d {

enum class ENTITY_TYPE
{
  EMPTY,

  // general categories,
  // however, this does not mean you could not attach a
  // component to an enemy to turn it in to a projectile?
  // i dunno ecs is weird

  // actors
  ENEMY,
  PLAYER,
  SHOPKEEPER,

  // solids
  WALL,

  // melee
  SWORD,
  FIRE_SWORD,
  CROSSBOW,
  SHIELD,

  // ballistics
  ARROW,
  BOLT,
  STONE,

  // consumable items
  POTION,
  SCROLL_MAGIC_MISSILE,
  SCROLL_FIREBALL,
  SCROLL_CONFUSION,

  // misc...
  FREE_CURSOR,

  COUNT
};

entt::entity
create_item(entt::registry& r, const ENTITY_TYPE& type, const entt::entity& parent);
void
create_item(entt::registry& r, const entt::entity& e, const ENTITY_TYPE& type, const entt::entity& parent);

void
create_renderable(entt::registry& r, const entt::entity& e, const ENTITY_TYPE& type);

void
remove_renderable(entt::registry& r, const entt::entity& e);

entt::entity
create_gameplay(entt::registry& r, const ENTITY_TYPE& type);
void
create_gameplay(entt::registry&, const entt::entity& e, const ENTITY_TYPE& type);

} // namespace game2d
