#pragma once

#include "engine/colour/colour.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

constexpr int n_players = 4;

const engine::SRGBColour default_player_colours[n_players] = {
  hex_to_srgb("#E8AE58"), // orangeish
  hex_to_srgb("#42B14F"), // green
  hex_to_srgb("#D075CE"), // pink
  hex_to_srgb("#00A8EB"), // blue
};

} // namespace game2d