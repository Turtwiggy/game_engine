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

  // actors with only one type (so far)
  actor_player,

  // solids
  solid_wall,

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

[[nodiscard]] SpriteColourComponent
create_colour(const SINGLETON_ColoursComponent& colours, const EntityType& type);

} // namespace game2d
