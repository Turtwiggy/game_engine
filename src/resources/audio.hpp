#pragma once

#include <SDL2/SDL_mixer.h>
#include <entt/entt.hpp>

#include <string>
#include <vector>

namespace game2d {

enum class AvailableSfx
{
  woosh,
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

Music&
get_music(entt::registry& r, AvailableMusic type);

Sfx&
get_sfx(entt::registry& r, AvailableSfx type);

struct SINGLETON_AudioComponent
{
  // audio device
  int device = 0;

  std::vector<Sfx> sfx{
    { AvailableSfx::woosh, "assets/audio/usfx_1_4/WHOOSHES/Air/WHOOSH_Air_Blade_RR1_mono.wav" },
  };

  std::vector<Music> music{
    { AvailableMusic::menu, "assets/audio/chase.mp3" },
  };
};

} // namespace game2d