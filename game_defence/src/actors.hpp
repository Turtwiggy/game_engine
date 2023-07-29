#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <box2d/box2d.h>

namespace game2d {

enum class EntityType
{
  empty,
  actor_player,
  actor_enemy,
  actor_turret,
  actor_bullet,

  // misc
  spawner,
  camera,
  line,

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

[[nodiscard]] entt::entity
create_gameplay(entt::registry& r, b2World& world, const EntityType& type);

} // namespace game2d
