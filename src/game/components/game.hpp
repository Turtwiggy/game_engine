#pragma once

#include "engine/maths/maths.hpp"

namespace game2d {

struct SINGLETON_GamePausedComponent
{
  bool paused = false;
};

struct SINGLETON_GameOverComponent
{
  bool over = false;
};

struct SINGLETON_ResourceComponent
{
  engine::RandomState rnd;
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

} // namespace game2d