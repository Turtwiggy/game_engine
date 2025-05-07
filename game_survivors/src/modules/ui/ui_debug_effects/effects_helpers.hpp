#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

void
spawn_fx(entt::registry& r, std::string name, glm::vec2 pos, glm::vec2 size);

} // namespace game2d