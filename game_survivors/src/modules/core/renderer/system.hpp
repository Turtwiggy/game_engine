#pragma once

#include <entt/fwd.hpp>

namespace game2d {

void
init_render_system(const glm::vec2 screen_wh, entt::registry& r);

void
update_render_system(entt::registry& r, const float dt, const glm::vec2& mouse_pos);

void
end_frame_render_system(entt::registry& registry);

} // namespace game2d
