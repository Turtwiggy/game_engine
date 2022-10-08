#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <vector>

namespace game2d {

enum class AvailableSfx
{
  // woosh,
  none,
};

enum class AvailableMusic
{
  menu,
};

struct Music
{
  AvailableMusic type;
  std::string path;
  Mix_Music* data = NULL;
};

struct Sfx
{
  AvailableSfx type;
  std::string path;
  Mix_Chunk* data = NULL;
};

struct SINGLETON_AudioComponent
{
  // audio device
  int device = 0;

  std::vector<Sfx> sfx{
    //{ AvailableSfx::woosh, "assets/audio/usfx_1_4/WHOOSHES/Air/WHOOSH_Air_Blade_RR1_mono.wav" },
  };

  std::vector<Music> music{
    { AvailableMusic::menu, "assets/audio/chase.mp3" },
  };
};

Music&
get_music(SINGLETON_AudioComponent& audio, AvailableMusic type);

Sfx&
get_sfx(SINGLETON_AudioComponent& audio, AvailableSfx type);


} // namespace game2d