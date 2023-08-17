#pragma once

#include <SDL2/SDL.h>

#include <string>
#include <vector>

namespace game2d {

struct ProfileResult
{
  std::string name;
  float ms = 0.0f;
};

struct SINGLETON_Profiler
{
  std::vector<ProfileResult> update_results;
  std::vector<ProfileResult> fixed_update_results;
};

} // namespace game2d