#pragma once

#include "AL/al.h"

#include <string>
#include <vector>

namespace game2d {

struct AudioRequestPlayEvent
{
  std::string tag;
  int position_x = 0;
  int position_y = 0;
  int position_z = 0;
};

struct AudioPlayingEvent
{
  ALuint source_id;
};

// sources can be positioned and played.
// source sound is determined position and
// orientation relative to the listener object.
struct AudioSource
{
  ALuint source_id = 0; // should be generated
};

struct AudioListener
{
  bool placeholder = true;
};

struct Sound
{
  std::string tag;
  std::string path;
  ALuint loaded_id = 0;
};

struct SINGLETON_AudioComponent
{
  std::vector<Sound> sounds;

  float master_volume = 1.0f;
};

} // namespace game2d