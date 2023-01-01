#pragma once

#include "components/app.hpp"

#include <glm/glm.hpp>

namespace game2d {

[[nodiscard]] glm::ivec2
mouse_position_in_worldspace(GameEditor& editor, Game& game);

} // namespace game2d