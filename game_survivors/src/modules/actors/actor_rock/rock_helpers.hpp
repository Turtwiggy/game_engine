#pragma once

#include <entt/fwd.hpp>

#include "engine/colour/colour.hpp"

namespace game2d {

void
generate_rocks(entt::registry& r, const float cutoff);

engine::SRGBColour
lerp_colour(engine::SRGBColour a, engine::SRGBColour b, float percent);

void
generate_island_interior(entt::registry& r);

void
generate_island_life__base_island(entt::registry& r);

void
generate_island_life__other_islands(entt::registry& r);

} // namespace game2d