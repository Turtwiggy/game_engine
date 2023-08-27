#pragma once

#include "resources/colours.hpp"
#include "sprites/components.hpp"

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class EntityType
{
  empty,

  // actors with only one type (so far)
  actor_hearth,
  actor_player,
  actor_spawner,
  actor_turret,
  actor_pickup_xp,
  actor_pickup_zone,

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
