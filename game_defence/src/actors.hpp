#pragma once

#include "sprites/components.hpp"

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <optional>
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

  // overworld
  actor_breach_charge,
  actor_spaceship,
  actor_capsule,
  actor_cargo,

  // dungeon
  actor_dungeon,

  // solids
  solid_wall,

  weapon_shotgun,
  // weapon_bow,
  // weapon_pistol,
  // weapon_sniper,
  bullet_default,
  bullet_bouncy,
  // bullet_bow,

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
create_gameplay(entt::registry& r,
                const EntityType& type,
                const glm::vec2& position,
                const std::optional<glm::vec2> size = std::nullopt);

void
add_particles(entt::registry& r, const entt::entity parent);

} // namespace game2d
