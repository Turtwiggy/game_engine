#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <vector>

namespace game2d {

// classes external System() should emplace

struct AudioRequestPlayEvent
{
  std::string tag;
  bool looping = false;
};

struct AudioCompleteEvent
{
  std::string tag;
};

// classes to support System()

enum class AudioSourceState
{
  FREE,
  PLAYING,
};

enum class SoundType
{
  BACKGROUND,
  SFX,
};

struct AudioSource
{
  int channel = -1;
  AudioSourceState state = AudioSourceState::FREE;
  SoundType sound_type = SoundType::BACKGROUND;
  std::string sound = "";

  AudioSource() = default;
  AudioSource(const int c)
    : channel(c) {};
};

struct AudioListener
{
  bool placeholder = true;
};

struct Sound
{
  std::string tag;
  std::string path;
  SoundType type = SoundType::BACKGROUND;

  // ALuint result;
  Mix_Chunk* buffer = nullptr;
};

struct SINGLE_AudioComponent
{
  std::vector<Sound> sounds;
  bool loaded = false;

  // set after requesting channels
  int max_audio_sources = -1;

  // volumes between 0 and 1
  float volume_master = 1.0f;
  float volume_music = 1.0f;
  float volume_sfx = 1.0f;

  bool refresh_devices = true;
  std::vector<std::string> devices;

  int captured_device_id = -1;
};

} // namespace game2d