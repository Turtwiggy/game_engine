#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <string>
#include <vector>

namespace game2d {

enum class AvailableSound
{
  menu,
  player_got_hit,
  player_attacked,
};

enum class SoundType
{
  RecieveDamage,
  GiveDamage,
};

struct Sound
{
  AvailableSound type;
  std::string path;
  // set after loading
  ALuint loaded_id = 0;
};

struct SINGLETON_AudioComponent
{
  // audio device
  // int device = 0;

  std::vector<Sound> sounds{
    //
    { AvailableSound::menu, "assets/audio/usfx_1_4/WEAPONS/Melee/Hammer/HAMMER_Hit_Body_stereo.wav" },
    { AvailableSound::player_got_hit,
      "assets/audio/usfx_1_4/VOICES/Martial_Arts_Male/VOICE_Martial_Art_Shout_03_mono.wav" },
    { AvailableSound::player_attacked, "assets/audio/usfx_1_4/WEAPONS/Melee/Hammer/HAMMER_Hit_Body_stereo.wav" }
  };
};

ALuint
get_sound_id(const SINGLETON_AudioComponent& audio, const AvailableSound type);

} // namespace game2d