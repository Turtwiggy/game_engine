#pragma once

// other lib headers
#include <SDL_mixer.h>

// c++ lib headers
#include <map>
#include <memory>
#include <string>

namespace game2d {

struct SINGLETON_AudioComponent
{
  std::map<std::string, Mix_Chunk*> bgm;
  std::map<std::string, std::vector<Mix_Chunk*>> sfx;
};

} // namespace game2d