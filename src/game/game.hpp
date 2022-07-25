#pragma once

// engine headers
#include "engine/app/application.hpp"

// other lib headers
#include "entt/entt.hpp"
#include "glm/glm.hpp"

namespace game2d {

void
init(entt::registry& registry, glm::ivec2 screen_wh);

void
fixed_update(entt::registry& registry, engine::Application& app, float fixed_dt);

void
update(entt::registry& registry, engine::Application& app, float dt);

} // namespace game2d