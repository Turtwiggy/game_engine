#pragma once

#include "resources/audio.hpp"

namespace game2d {

// sources can be positioned and played.
// source sound is determined position and
// orientation relative to the listener object.
struct AudioSource
{
  ALuint source_id = 0; // should be generated
  std::vector<AvailableSound> audio_selection;
  std::vector<AvailableSound> wants_to_play;
};

struct AudioListener
{
  bool placeholder = true;
};

} // namespace game2d