#pragma once

#include "engine/colour.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

enum class EntityType
{
  empty,

  // general categories,
  // however, this does not mean you could not attach a
  // component to an enemy to turn it in to a projectile?
  // i dunno ecs is weird

  // actors
  enemy,
  player,
  shopkeeper,

  // tile types
  wall,
  floor,

  // melee
  sword,
  fire_sword,
  crossbow,
  shield,

  // ballistics
  arrow,
  bolt,
  stone,

  // consumable items
  potion,
  scroll_magic_missile,
  scroll_fireball,
  scroll_confusion,

  // misc...
  free_cursor,
  aim_line,

  count
};

entt::entity
create_item(entt::registry& r, const EntityType& type, const entt::entity& parent);
// void
// create_item(entt::registry& r, const entt::entity& e, const EntityType& type, const entt::entity& parent);

TransformComponent
create_transform(entt::registry& r, const entt::entity& e);

SpriteComponent
create_sprite(entt::registry& r, const entt::entity& e, const EntityType& type);

void
create_renderable(entt::registry& r, const entt::entity& e, const EntityType& type);

entt::entity
create_gameplay(entt::registry& r, const EntityType& type);
void
create_gameplay(entt::registry&, const entt::entity& e, const EntityType& type);

} // namespace game2d
