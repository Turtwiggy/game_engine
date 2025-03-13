#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

entt::entity
create_snake(entt::registry& r);

void
update_snake(entt::registry& r, glm::vec2 mouse_pos, float dt);

} // namespace game2d