#pragma once

#include <SDL2/SDL_scancode.h>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

//
// "tag" components i.e. specific entities
//

struct PlayerComponent
{
  // movement
  int speed = 100;

  // input
  SDL_Scancode W = SDL_SCANCODE_W;
  SDL_Scancode A = SDL_SCANCODE_A;
  SDL_Scancode S = SDL_SCANCODE_S;
  SDL_Scancode D = SDL_SCANCODE_D;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerComponent, speed, W, A, S, D);
};

} // namespace game2d