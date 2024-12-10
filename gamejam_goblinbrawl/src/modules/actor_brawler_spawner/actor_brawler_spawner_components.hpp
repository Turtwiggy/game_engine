#pragma once

#include "engine/colour/colour.hpp"
#include <SDL_scancode.h>
#include <entt/entt.hpp>

namespace game2d {

struct BrawlerSpawner
{
  bool spawns_ai = false;
  SDL_Scancode key = SDL_SCANCODE_SPACE;
  engine::SRGBColour colour;
  int team_idx = 0;
};

} // namespace game2d