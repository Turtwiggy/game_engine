#pragma once

#include <SDL2/SDL_scancode.h>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct PlayerComponent
{
  // movement
  // int speed = 100;
  // bool able_to_move = true;
  // int milliseconds_move_cooldown = 0;

  // input
  SDL_Scancode W = SDL_SCANCODE_W;
  SDL_Scancode A = SDL_SCANCODE_A;
  SDL_Scancode S = SDL_SCANCODE_S;
  SDL_Scancode D = SDL_SCANCODE_D;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerComponent, W, A, S, D);
};

struct InputComponent
{
  glm::ivec2 dir;
};

} // namespace game2d