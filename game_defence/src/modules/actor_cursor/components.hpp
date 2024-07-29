#pragma once

#include <glm/glm.hpp>
#include <optional>

namespace game2d {

struct CursorComponent
{
  // the problem with storing grid_index on the cursor,
  // is loads of different systems could want info
  // on grids of different sizes.
  // std::optional<int> grid_index = std::nullopt;

  std::optional<glm::ivec2> click_location;
};

} // namespace game2d