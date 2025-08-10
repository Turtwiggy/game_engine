#pragma once

#include <entt/fwd.hpp>

namespace game2d {

void
draw_popup(entt::registry& r, const ImVec2 tl, std::string text, ImFont* font);

} // namespace game2d