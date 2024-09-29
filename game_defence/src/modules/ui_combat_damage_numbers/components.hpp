#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct TimedEntry
{
  std::string data;
  float time_left = 0.0f;
  float time_left_max = 2.0f;
};

// append entries to the end,
// but visually, they are shown first (as most recent)
// if the entry at the beginning is out of time_left, remove it
struct UI_BufferComponent
{
  std::vector<TimedEntry> entries;
};

} // namespace game2d