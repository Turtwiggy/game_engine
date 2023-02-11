#include "helpers.hpp"

#include <algorithm>

namespace game2d {

ALuint
get_sound_id(const SINGLETON_AudioComponent& audio, const std::string& tag)
{
  const auto& res = audio.sounds;
  auto it = std::find_if(res.begin(), res.end(), [&tag](const auto& a) { return a.tag == tag; });
  return it->loaded_id;
}

} // namespace game2d