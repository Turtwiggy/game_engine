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

enum class SoundType
{
  BACKGROUND,
  SFX,
};

struct Sound
{
  std::string tag;
  std::string path;
  SoundType type = SoundType::BACKGROUND;

  // ALuint result;
  Mix_Chunk* buffer;
};

struct SINGLETON_AudioComponent
{
  std::vector<Sound> sounds;
  bool loaded = false;

  bool mute_all = false;
  bool mute_sfx = false;
  // float master_volume = 1.0f;

  bool refresh_devices = true;
  std::vector<std::string> devices;

  int captured_device_id = -1;
};

} // namespace game2d