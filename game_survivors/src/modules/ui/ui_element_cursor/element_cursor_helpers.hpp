#pragma once

#include "element_cursor_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
draw_cursor(entt::registry& r, UiCursorComponent& cursor_c, const ImVec2 tl, float dt);

} // namespace game2d