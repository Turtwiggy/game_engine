#pragma once

#include <SDL2/SDL_mixer.h>

#include <string>
#include <vector>

namespace game2d {

struct Music
{
  std::string name;
  std::string path;
  Mix_Music* data = NULL;
};

struct Sfx
{
  std::string name;
  std::string path;
  Mix_Chunk* data = NULL;
};

struct SINGLETON_AudioComponent
{
  // audio device
  int device = 0;

  std::vector<Sfx> sfx{
    { "woosh", "assets/audio/usfx_1_4/WHOOSHES/Air/WHOOSH_Air_Blade_RR1_mono.wav" },
  };

  std::vector<Music> music{
    { "menu", "assets/audio/chase.mp3" },
  };
};

} // namespace game2d