#pragma once

#include "colour/colour.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

enum class EntityType
{
  empty,
  actor_player,
  camera,

  count
};

struct EntityTypeComponent
{
  EntityType type;
};

// using json = nlohmann::json;
// void
// to_json(json& j, const EntityTypeComponent& et);
// void
// from_json(const json& j, EntityTypeComponent& et);

// [[nodiscard]] TransformComponent
// create_transform();
// [[nodiscard]] SpriteComponent
// create_sprite(GameEditor& editor, const EntityType& type);
// [[nodiscard]] SpriteColourComponent
// create_colour(GameEditor& editor, const EntityType& type);

// void
// create_renderable(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type);
// void
// create_renderable(GameEditor& editor, Game& g, const entt::entity& e, const EntityType& type);

// [[nodiscard]] entt::entity
// create_item(GameEditor& editor, entt::registry& r, const EntityType& type, const entt::entity& parent);
// [[nodiscard]] entt::entity
// create_item(GameEditor& editor, Game& game, const EntityType& type, const entt::entity& parent);

// [[nodiscard]] entt::entity
// create_gameplay(GameEditor& editor, entt::registry& r, const EntityType& type);
// [[nodiscard]] entt::entity
// create_gameplay(GameEditor& editor, Game& game, const EntityType& type);

// void
// create_gameplay_existing_entity(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type);
// void
// create_gameplay_existing_entity(GameEditor& editor, Game& game, const entt::entity& e, const EntityType& type);

} // namespace game2d
