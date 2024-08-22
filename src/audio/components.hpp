#pragma once

#include <SDL2/SDL_mixer.h>
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
  Mix_Chunk* buffer = nullptr;
};

struct SINGLETON_AudioComponent
{
  std::vector<Sound> sounds;
  bool loaded = false;

  // set after requesting channels
  int max_audio_sources = -1;

  // set volume as 10%. try not to blow out ear drums on launch.
  float volume_user = 0.1f; // between 0 and 1
  int volume_internal = static_cast<int>(MIX_MAX_VOLUME * volume_user);

  bool mute_all = false;
  bool mute_sfx = false;
  // float master_volume = 1.0f;

  bool refresh_devices = true;
  std::vector<std::string> devices;

  int captured_device_id = -1;
};

} // namespace game2d