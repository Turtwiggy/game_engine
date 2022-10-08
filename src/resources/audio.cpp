#include "audio.hpp"

#include <algorithm>

namespace game2d {

Music&
get_music(SINGLETON_AudioComponent& audio, AvailableMusic type)
{
  auto& res = audio.music;
  auto it = std::find_if(res.begin(), res.end(), [&type](const auto& a) { return a.type == type; });
  return *it;
};

Sfx&
get_sfx(SINGLETON_AudioComponent& audio, AvailableSfx type)
{
  auto& res = audio.sfx;
  auto it = std::find_if(res.begin(), res.end(), [&type](const auto& a) { return a.type == type; });
  return *it;
};

} // namespace game2d