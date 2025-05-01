#pragma once

#include <entt/fwd.hpp>

#include "engine/colour/colour.hpp"

namespace game2d {

void
generate_rocks(entt::registry& r, const float cutoff);

engine::SRGBColour
lerp_colour(engine::SRGBColour a, engine::SRGBColour b, float percent);

} // namespace game2d