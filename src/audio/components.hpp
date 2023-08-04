#pragma once

#include "AL/al.h"
#include "AL/alc.h"

#include <string>
#include <vector>

namespace game2d {

// classes external System() should emplace

struct AudioRequestPlayEvent
{
  std::string tag;
};

// classes to support System()

enum class AudioSourceState
{
  FREE,
  PLAYING,
};

// sources can be positioned and played.
// source sound is determined position and
// orientation relative to the listener object.
struct AudioSource
{
  ALuint source_id = 0; // generated
  AudioSourceState state = AudioSourceState::FREE;
};

struct AudioListener
{
  bool placeholder = true;
};

struct Sound
{
  std::string tag;
  std::string path;
  ALuint result;
};

struct SINGLETON_AudioComponent
{
  std::vector<Sound> sounds;

  float master_volume = 1.0f;

  bool refresh_devices = true;
  std::vector<std::string> playback_devices;
  std::vector<std::string> captured_devices;
};

} // namespace game2d