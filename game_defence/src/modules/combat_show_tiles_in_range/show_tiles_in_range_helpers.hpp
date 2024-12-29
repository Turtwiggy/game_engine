#pragma once

#include "engine/map/components.hpp"
#include "modules/actor_player/components.hpp"


#include <entt/entt.hpp>

namespace game2d {

entt::entity
get_equipped_gun(entt::registry& r, const entt::entity e);

int
get_damage_for_item(entt::registry& r, entt::entity e);

int
get_damage_for_equipped_item(entt::registry& r, const entt::entity e);

//
//
//

std::vector<glm::ivec2>
get_tiles_for_knife(entt::registry& r, const MapComponent& map_c, const glm::ivec2& pos);

std::vector<glm::ivec2>
get_tiles_in_line(entt::registry& r,
                  const MapComponent& map_c,
                  const glm::ivec2& pos,
                  const glm::vec2 look_dir,
                  const int length);

std::vector<glm::ivec2>
get_tiles_for_shotgun(entt::registry& r, const MapComponent& map_c, const glm::ivec2& pos, const glm::vec2 look_dir);

} // namespace game2d