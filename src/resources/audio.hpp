#pragma once

#include <string>
#include <vector>

namespace game2d {

enum class AvailableSound
{
  // woosh,
  menu,
};

struct Sound
{
  AvailableSound type;
  std::string path;
};

struct SINGLETON_AudioComponent
{
  // audio device
  int device = 0;

  std::vector<Sound> sounds{
    { AvailableSound::menu, "assets/audio/chase.mp3" },
  };
};

} // namespace game2d