#pragma once

#include "engine/colour/colour.hpp"

#include <entt/entt.hpp>

namespace game2d {

engine::SRGBColour
hex_to_srgb(const std::string& hex);

} // namespace game2d