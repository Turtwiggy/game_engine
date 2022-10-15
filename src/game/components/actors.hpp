#pragma once

#include "engine/colour.hpp"
#include "game/components/app.hpp"
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
  enemy_troll,
  enemy_orc,
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
  scroll_damage_nearest,

  // misc...
  camera,
  free_cursor,

  count
};

struct EntityTypeComponent
{
  EntityType type;
};

using json = nlohmann::json;
void
to_json(json& j, const EntityTypeComponent& et);
void
from_json(const json& j, EntityTypeComponent& et);

entt::entity
create_item(GameEditor& editor, Game& game, const EntityType& type, const entt::entity& parent);
// void
// create_item(entt::registry& r, const entt::entity& e, const EntityType& type, const entt::entity& parent);

TransformComponent
create_transform(entt::registry& r, const entt::entity& e);
SpriteComponent
create_sprite(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type);
SpriteColourComponent
create_colour(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type);

void
create_renderable(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type);

entt::entity
create_gameplay(GameEditor& editor, Game& game, const EntityType& type);
void
create_gameplay(GameEditor& editor, Game& game, const entt::entity& e, const EntityType& type);

} // namespace game2d
