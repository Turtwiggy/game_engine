#pragma once

#include <SDL_mixer.h>
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

struct AudioSource
{
  int channel = -1;
  AudioSourceState state = AudioSourceState::FREE;

  AudioSource() = default;
  AudioSource(const int c)
    : channel(c){};
};

struct AudioListener
{
  bool placeholder = true;
};

struct Sound
{
  std::string tag;
  std::string path;

  // ALuint result;
  Mix_Chunk* buffer;
};

struct SINGLETON_AudioComponent
{
  std::vector<Sound> sounds;

  bool all_mute = false;
  float master_volume = 1.0f;

  bool refresh_devices = true;
  std::vector<std::string> devices;

  int captured_device_id = -1;
};

} // namespace game2d