#pragma once

#include "colour/colour.hpp"
#include "lifecycle/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class EntityType : std::size_t
{
  // warning: changing the positions of these elements
  // could break the level-editor system
  // as the order is used for unique-ids when saved to disk
  // probably should do something else

  empty_with_transform,
  actor_breach_charge,
  actor_dungeon,
  actor_jetpack_player,
  actor_space_ship,
  actor_space_capsule,
  actor_space_cargo,
  bullet_bouncy,
  bullet_default,
  particle,
  solid_wall,
  weapon_shotgun,

  count
};

struct EntityTypeComponent
{
  EntityType type = EntityType::empty_with_transform;

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

struct EntityDescription
{
  glm::vec2 pos{ 0, 0 };
  glm::vec2 size{ 32, 32 };
  std::string sprite;
  engine::SRGBColour colour{ 1.0f, 1.0f, 1.0f, 1.0f };
};

template<EntityType E>
struct Actor
{
  static constexpr EntityType type = E;
};

class Factory_BaseActor
{
public:
  static entt::entity create(entt::registry& r, const EntityType& type, const EntityDescription& desc);
};

} // namespace game2d