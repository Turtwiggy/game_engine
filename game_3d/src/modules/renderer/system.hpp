#pragma once

#include "app/application.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
init_renderer_system(const engine::SINGLETON_Application& app, entt::registry& r);

void
update_renderer_system(engine::SINGLETON_Application& app, entt::registry& r, const float dt);

} // namespace game2d