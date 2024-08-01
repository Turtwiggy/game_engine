#pragma once

#include "sprites/components.hpp"

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <string>

namespace game2d {

enum class EntityType
{
  // warning: changing the positions of these elements
  // could break the level-editor system
  // as the order is used for unique-ids when saved to disk
  // probably should do something else

  // empty_no_transform, // use create_empty instead
  empty_with_transform,
  empty_with_physics,

  // actors with only one type (so far)
  // actor_hearth,
  // actor_spawner,
  // actor_turret,

  // solids
  solid_wall,

  // overworld
  actor_player, // overworld player
  actor_enemy_patrol,

  // dungeon
  actor_unit_rtslike, // selectable and moveable
  actor_barrel,
  // actor_barricade,
  weapon_shotgun,
  // weapon_bow,
  // weapon_pistol,
  // weapon_sniper,
  bullet_default,
  bullet_bouncy,
  // bullet_bow,
  // enemy_dummy,
  // enemy_grunt,
  // enemy_ranged,

  // pickups
  actor_pickup_xp,
  actor_pickup_doubledamage,

  // misc
  cursor,
  particle,

  count
};

struct EntityTypeComponent
{
  EntityType type = EntityType::count;

  // NLOHMANN_DEFINE_TYPE_INTRUSIVE(EntityTypeComponent, type);
};
inline void
to_json(nlohmann::json& j, const EntityTypeComponent& et)
{
  j = nlohmann::json{ { "type", static_cast<int>(et.type) } };
};
inline void
from_json(const nlohmann::json& j, EntityTypeComponent& et)
{
  j.at("type").get_to(et.type);
};

SpriteComponent
create_sprite(entt::registry& r, const std::string& sprite, const EntityType& type);

[[nodiscard]] entt::entity
create_transform(entt::registry& r);

// have to set position for physics entities
[[nodiscard]] entt::entity
create_gameplay(entt::registry& r, const EntityType& type, const glm::vec2& position);

} // namespace game2d
