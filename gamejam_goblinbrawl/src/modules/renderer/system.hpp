#pragma once

#include "engine/app/application.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
init_render_system(const engine::SINGLE_Application& app, entt::registry& r);

void
update_render_system(entt::registry& r, const float dt, const glm::vec2& mouse_pos);

void
end_frame_render_system(entt::registry& registry);

} // namespace game2d
