#pragma once

#include "components.hpp"

#include "maths/grid.hpp"

namespace game2d {

// digital differential analysis algorithm

DDAOutput
dda_raycast(const DDAInput& input, const std::vector<int>& map);

} // namespace game2d