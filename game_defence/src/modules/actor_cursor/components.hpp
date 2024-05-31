#pragma once

#include <glm/glm.hpp>
#include <optional>

namespace game2d {

struct CursorComponent
{
  std::optional<int> grid_index = std::nullopt;
  std::optional<glm::ivec2> click_location;
};

} // namespace game2d