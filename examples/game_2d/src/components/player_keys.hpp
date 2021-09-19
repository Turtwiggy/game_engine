#pragma once

// other lib headers
#include <SDL2/SDL_scancode.h>

namespace game2d {

struct PlayerKeys
{
  bool use_keyboard = true;

  // keyboard
  SDL_Scancode w = SDL_SCANCODE_W;
  SDL_Scancode s = SDL_SCANCODE_S;
  SDL_Scancode a = SDL_SCANCODE_A;
  SDL_Scancode d = SDL_SCANCODE_D;
  SDL_Scancode key_boost = SDL_SCANCODE_LSHIFT;
  SDL_Scancode key_pause = SDL_SCANCODE_P;
  SDL_Scancode key_camera_up = SDL_SCANCODE_UP;
  SDL_Scancode key_camera_down = SDL_SCANCODE_DOWN;
  SDL_Scancode key_camera_left = SDL_SCANCODE_LEFT;
  SDL_Scancode key_camera_right = SDL_SCANCODE_RIGHT;
  SDL_Scancode key_camera_follow_player = SDL_SCANCODE_Q;

  // controller input
  // float l_analogue_x = 0.0f;
  // float l_analogue_y = 0.0f;
  // float r_analogue_x = 0.0f;
  // float r_analogue_y = 0.0f;

  // key state
  // float angle_around_player = 0.0f;
  // bool pause_held = false;
  // bool pause_down = false;
  // bool shoot_held = false;
  // bool shoot_down = false;
  // bool boost_held = false;
  // bool boost_down = false;
};

} // namespace game2d