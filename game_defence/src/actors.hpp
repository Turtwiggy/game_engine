#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

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

using json = nlohmann::json;
void
to_json(json& j, const EntityTypeComponent& et);
void
from_json(const json& j, EntityTypeComponent& et);

void
create_renderable(entt::registry& r, const entt::entity& e, const EntityType& type);

[[nodiscard]] entt::entity
create_gameplay(entt::registry& r, const EntityType& type);

void
create_gameplay_existing_entity(entt::registry& r, const entt::entity& e, const EntityType& type);

} // namespace game2d
