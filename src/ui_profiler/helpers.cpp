#include "helpers.hpp"

#include <SDL2/SDL.h>

namespace game2d {

Timer::Timer()
{
  start = SDL_GetPerformanceCounter();
};

float
Timer::elapsed() const
{
  return (SDL_GetPerformanceCounter() - start) / float(SDL_GetPerformanceFrequency());
};

float
Timer::elapsed_ms() const
{
  return elapsed() * 1000.0f;
};

ScopedTime::ScopedTime(SINGLETON_Profiler* p, const std::string& n, bool fu)
  : profiler(p)
  , name(n)
  , fixed_update(fu){};

ScopedTime::~ScopedTime()
{
  if (fixed_update)
    profiler->fixed_update_results.push_back({ name, t.elapsed_ms() });
  else
    profiler->update_results.push_back({ name, t.elapsed_ms() });
};

ScopedTime
time_scope(SINGLETON_Profiler* p, const std::string& name, bool fixed_update)
{
  return ScopedTime(p, name, fixed_update);
};

} // namespace game2d