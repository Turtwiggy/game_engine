#pragma once

#include "components.hpp"

#include <string>
#include <vector>

namespace game2d {

// A generic timer
struct Timer
{
  Timer();
  float elapsed() const;
  float elapsed_ms() const;

private:
  uint64_t start;
};

// Adds an instance to the profiler
struct ScopedTime
{
  ScopedTime(SINGLETON_Profiler* p, const std::string& n, bool fixed_update = false);
  ~ScopedTime();

  SINGLETON_Profiler* const profiler;
  std::string name;
  Timer t;
  bool fixed_update = false;
};

ScopedTime
time_scope(SINGLETON_Profiler* p, const std::string& name, bool fixed_update = false);

} // namespace game2d