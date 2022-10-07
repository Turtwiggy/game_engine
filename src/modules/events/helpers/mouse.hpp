#pragma once

#include "game/components/app.hpp"

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

// mouse
// note: _down will occur on the same frame as first _held
// note: _release will occur on the frame after last _held
[[nodiscard]] bool
get_mouse_lmb_held();

[[nodiscard]] bool
get_mouse_rmb_held();

[[nodiscard]] bool
get_mouse_mmb_held();

[[nodiscard]] bool
get_mouse_lmb_press();

[[nodiscard]] bool
get_mouse_rmb_press();

[[nodiscard]] bool
get_mouse_mmb_press();

[[nodiscard]] bool
get_mouse_lmb_release();

[[nodiscard]] bool
get_mouse_rmb_release();

[[nodiscard]] bool
get_mouse_mmb_release();

[[nodiscard]] glm::ivec2
get_mouse_pos();

[[nodiscard]] glm::ivec2
mouse_position_in_worldspace(GameEditor& editor, Game& game);

} // namespace game2d