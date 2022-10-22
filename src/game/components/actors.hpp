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

  actor_player,
  actor_troll,
  actor_orc,
  actor_shopkeeper,

  tile_type_wall,
  tile_type_floor,
  tile_type_exit,

  // equipment
  sword,
  fire_sword,
  crossbow,
  arrow,
  bolt,
  stone,
  shield,
  armour_leather,
  armour_chainmail,

  // consumable
  potion,
  scroll_damage_nearest,
  scroll_damage_selected_on_grid,

  // misc...
  camera,
  free_cursor,
  grid_cursor,

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
create_item(GameEditor& editor, entt::registry& r, const EntityType& type, const entt::entity& parent);
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
create_gameplay(GameEditor& editor, entt::registry& r, const EntityType& type);

void
create_gameplay_existing_entity(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type);

} // namespace game2d
