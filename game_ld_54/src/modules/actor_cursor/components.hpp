#pragma once

#include <optional>

namespace game2d {

struct CursorComponent
{
  std::optional<int> grid_index = std::nullopt;
};

} // namespace game2d