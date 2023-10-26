#pragma once

#include "resources/colours.hpp"
#include "sprites/components.hpp"

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class EntityType
{
  // warning: changing the positions of these elements
  // could break the level-editor system
  // as the order is used for unique-ids when saved to disk
  // probably should do something else

  // transform-only
  empty,
  empty_with_physics,

  // actors with only one type (so far)
  actor_hearth,
  actor_player,
  actor_spawner,
  actor_turret,

  // in progress
  // actor_pickup_xp,
  // actor_pickup_zone,
  actor_dispencer,
  actor_customer_area,

  // items
  item,

  // solids
  solid_wall,

  // actor_weapons,
  weapon_bow,
  // weapon_pistol,
  // weapon_sniper,
  // weapon_shotgun,

  // actor_bullets,
  bullet_default,
  bullet_bow,

  // actor_enemy,
  enemy_grunt,
  enemy_sniper,
  enemy_shotgunner,

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

[[nodiscard]] entt::entity
create_gameplay(entt::registry& r, const EntityType& type);

[[nodiscard]] SpriteComponent
create_sprite(entt::registry& r, const EntityType& type);

} // namespace game2d
