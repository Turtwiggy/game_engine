#pragma once

#include "components.hpp"

namespace game2d {

engine::SRGBColour
lerp_colour(engine::SRGBColour a, engine::SRGBColour b, float percent);

} // namespace game2d