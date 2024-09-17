#pragma once

#include "engine/colour/colour.hpp"

namespace game2d {

struct DefaultColour
{
  engine::SRGBColour colour{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct HoveredColour
{
  engine::SRGBColour colour{ 1.0f, 1.0f, 1.0f, 1.0f };
};

} // namespace game2d