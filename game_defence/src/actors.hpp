#pragma once

#include "resources/colours.hpp"
#include "sprites/components.hpp"

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class EntityType
{
  empty,
  actor_arrow,
  actor_bow,
  actor_bullet,
  actor_enemy,
  actor_hearth,
  actor_player,
  actor_spawner,
  actor_turret,

  pickup_zone,
  pickup_xp,

  // misc
  cursor,
  camera,
  line,
  particle,

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
create_gameplay(entt::registry& r, const EntityType& type);

[[nodiscard]] SpriteColourComponent
create_colour(const SINGLETON_ColoursComponent& colours, const EntityType& type);

} // namespace game2d
