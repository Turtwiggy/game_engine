#pragma once

#include "colour/colour.hpp"
#include "components/app.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

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
  actor_bat,
  actor_troll,
  actor_shopkeeper,

  tile_type_wall,
  tile_type_floor,
  tile_type_exit,

  // equipment
  sword,
  shield,
  // crossbow,
  // bolt,
  // fire_sword,
  // arrow,
  // stone,
  // armour_leather,
  // armour_chainmail,

  // consumable
  potion,
  scroll_damage_nearest,
  // scroll_damage_selected_on_grid,

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

[[nodiscard]] TransformComponent
create_transform();
[[nodiscard]] SpriteComponent
create_sprite(GameEditor& editor, const EntityType& type);
[[nodiscard]] SpriteColourComponent
create_colour(GameEditor& editor, const EntityType& type);

void
create_renderable(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type);
void
create_renderable(GameEditor& editor, Game& g, const entt::entity& e, const EntityType& type);

[[nodiscard]] entt::entity
create_item(GameEditor& editor, entt::registry& r, const EntityType& type, const entt::entity& parent);
[[nodiscard]] entt::entity
create_item(GameEditor& editor, Game& game, const EntityType& type, const entt::entity& parent);

[[nodiscard]] entt::entity
create_gameplay(GameEditor& editor, entt::registry& r, const EntityType& type);
[[nodiscard]] entt::entity
create_gameplay(GameEditor& editor, Game& game, const EntityType& type);

void
create_gameplay_existing_entity(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type);
void
create_gameplay_existing_entity(GameEditor& editor, Game& game, const entt::entity& e, const EntityType& type);

} // namespace game2d