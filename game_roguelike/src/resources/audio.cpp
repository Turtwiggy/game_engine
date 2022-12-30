#include "audio.hpp"

#include <algorithm>

namespace game2d {

ALuint
get_sound_id(const SINGLETON_AudioComponent& audio, const AvailableSound type)
{
  const auto& res = audio.sounds;
  auto it = std::find_if(res.begin(), res.end(), [&type](const auto& a) { return a.type == type; });
  return it->loaded_id;
}

} // namespace game2d