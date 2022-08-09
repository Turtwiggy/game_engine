#pragma once

#include "resources/audio.hpp"

#include <SDL2/SDL_mixer.h>

#include <map>
#include <string>

namespace game2d {

struct SINGLETON_AudioComponent
{
  // audio device
  int device;

  // sounds
  const std::string sound_path = "assets/audio/usfx_1_4/WHOOSHES/Air/WHOOSH_Air_Blade_RR1_mono.wav";
  Mix_Chunk* sound;
};

} // namespace game2d