#pragma once

// other project headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

entt::entity
create_player(entt::registry& registry, glm::ivec2 pos);

} // namespace: game2d