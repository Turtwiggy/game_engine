
// header
#include "engine/tools/profiler.hpp"

// c system headers
#include <cassert>

// c++ standard library headers
#include <numeric>

namespace fightingengine {

uint8_t
Profiler::get_entry_index(int8_t offset) const
{
  return (current_entry + frames_data_live + offset) % frames_data_live;
}

void
Profiler::new_frame()
{
  auto& prevEntry = entries[current_entry];
  current_entry = (current_entry + 1) % frames_data_live;
  prevEntry.frame_end = entries[current_entry].frame_start =
    std::chrono::system_clock::now();
}

float
Profiler::get_time(const Stage& request) const
{
  auto& entry = entries[get_entry_index(-1)];
  auto& stage = entry.stages[(int)request];

  std::chrono::duration<float, std::milli> fltStart = stage._start - entry.frame_start;
  float startTimestamp = fltStart.count();

  std::chrono::duration<float, std::milli> fltEnd = stage._end - entry.frame_end;
  float endTimestamp = fltEnd.count();

  return endTimestamp - startTimestamp;
}

float
Profiler::get_average_time(const Stage& request) const
{
  float average = 0.0f;
  int valid_entries = 0;

  for (auto& entry : entries) {
    auto& delta_time = entry.stages[(int)request];

    std::chrono::duration<float, std::milli> fltStart =
      delta_time._start - entry.frame_start;
    float startTimestamp = fltStart.count();

    std::chrono::duration<float, std::milli> fltEnd =
      delta_time._end - entry.frame_start;
    float endTimestamp = fltEnd.count();

    float time = endTimestamp - startTimestamp;

    if (time <= 0.0f) {
      continue;
    }

    average += time;
    valid_entries++;
  }

  if (valid_entries == 0)
    return 0;

  return average / (float)valid_entries;
}

void
Profiler::begin(const Stage& stage)
{
  assert(frames_data_live < 255);
  auto& delta_time = entries[current_entry].stages[static_cast<uint8_t>(stage)];

  delta_time._start = std::chrono::system_clock::now();
  delta_time.scope_time_finalized = false;
}

void
Profiler::end(const Stage& stage)
{
  auto& delta_time = entries[current_entry].stages[static_cast<uint8_t>(stage)];
  assert(!delta_time.scope_time_finalized);

  delta_time._end = std::chrono::system_clock::now();
  delta_time.scope_time_finalized = true;
}

} // namespace fightingengine