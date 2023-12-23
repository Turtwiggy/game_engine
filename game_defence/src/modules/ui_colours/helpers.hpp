#pragma once

#include "colour/colour.hpp"

#include <entt/entt.hpp>

namespace game2d {

engine::LinearColour
get_lin_colour_by_tag(entt::registry& r, const std::string& tag);

engine::SRGBColour
get_srgb_colour_by_tag(entt::registry& r, const std::string& tag);

} // namespace game2d
