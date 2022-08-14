#pragma once

#include <SDL2/SDL_scancode.h>
#include <entt/entt.hpp>

namespace game2d {

struct SINGLETON_GameOverComponent
{
  bool over = false;
};

struct SINGLETON_AsteroidGameStateComponent
{
  float initial_asteroids = 20;
  float max_asteroids = 45;
  float time_between_asteroids = 0.25f;
  float time_since_last_asteroid = 0.25f;
  float asteroid_min_vel = -100.0f;
  float asteroid_max_vel = 100.0f;
  float score = 0.0f;
};

struct AsteroidComponent
{
  bool initialized = false;
  float spin_amount = 0.0f;
};

struct PlayerComponent
{
  // movement
  int speed = 0;

  // input
  SDL_Scancode W = SDL_SCANCODE_W;
  SDL_Scancode A = SDL_SCANCODE_A;
  SDL_Scancode S = SDL_SCANCODE_S;
  SDL_Scancode D = SDL_SCANCODE_D;
};

entt::entity
create_player(entt::registry& r);

entt::entity
create_enemy(entt::registry& r);

entt::entity
create_bullet(entt::registry& r);

} // namespace game2d
