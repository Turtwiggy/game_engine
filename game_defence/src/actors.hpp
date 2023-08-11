#pragma once

#include "resources/colours.hpp"
#include "sprites/components.hpp"

#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class EntityType
{
  empty,
  empty_with_physics,
  actor_player,
  actor_turret,
  actor_bullet,
  actor_hearth,
  actor_enemy,

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

[[nodiscard]] SpriteColourComponent
create_colour(const SINGLETON_ColoursComponent& colours, const EntityType& type);

} // namespace game2d
