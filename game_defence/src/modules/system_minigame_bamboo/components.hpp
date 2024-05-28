#pragma once

#include "events/components.hpp"
#include <vector>

namespace game2d {

struct SINGLE_MinigameBamboo
{
  std::vector<InputEvent> inputs;
  std::vector<InputEvent> combination;
  int last_processed_tick = 0;

  // difficulty scaling
  float time_left_base = 3.0f;
  float time_increase = 1.0f;
  float time_left = 3.0f;
  int combination_length = 3;
};

struct GenerateCombinationRequest
{
  bool placeholder = true;
};

} // namespace game2d