#pragma once

#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_scancode.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct PlayerComponent
{
  float time_between_bullets_left = 0.0f;
  entt::entity debug_gun_spot = entt::null;
  entt::entity pickup_area = entt::null;
  int picked_up_xp = 0;
};

struct KeyboardComponent
{
  // input
  SDL_Scancode W = SDL_SCANCODE_W;
  SDL_Scancode A = SDL_SCANCODE_A;
  SDL_Scancode S = SDL_SCANCODE_S;
  SDL_Scancode D = SDL_SCANCODE_D;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(KeyboardComponent, W, A, S, D);
};

struct ControllerComponent
{
  SDL_GameControllerAxis c_left_stick_x = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX;
  SDL_GameControllerAxis c_left_stick_y = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY;
  SDL_GameControllerAxis c_right_stick_x = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX;
  SDL_GameControllerAxis c_right_stick_y = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY;
  SDL_GameControllerAxis c_left_trigger = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT;
  SDL_GameControllerAxis c_right_trigger = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
  SDL_GameControllerButton c_start = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START;
  SDL_GameControllerButton c_a = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A;
  SDL_GameControllerButton c_b = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B;
  SDL_GameControllerButton c_x = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X;
  SDL_GameControllerButton c_y = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y;
  SDL_GameControllerButton c_dpad_up = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP;
  SDL_GameControllerButton c_dpad_down = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN;
  SDL_GameControllerButton c_dpad_left = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT;
  SDL_GameControllerButton c_dpad_right = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
  SDL_GameControllerButton c_l_bumper = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
  SDL_GameControllerButton c_r_bumper = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
};

struct InputComponent
{
  glm::vec2 stick_l;
  glm::vec2 stick_r;
};

} // namespace game2d