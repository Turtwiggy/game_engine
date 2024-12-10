#pragma once

#include "components.hpp"

#include "entt/entt.hpp"
#include <glm/glm.hpp>

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene& s);

// e.g. menu to overworld
void
move_to_scene_additive(entt::registry&, const Scene& s);

} // namespace game2d