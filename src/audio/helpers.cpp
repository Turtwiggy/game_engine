#include "helpers.hpp"

#include <algorithm>
#include <fmt/core.h>

namespace game2d {

Sound
get_sound(const SINGLETON_AudioComponent& audio, const std::string& tag)
{
  const auto& res = audio.sounds;
  auto it = std::find_if(res.begin(), res.end(), [&tag](const auto& a) { return a.tag == tag; });
  return (*it);
};

} // namespace game2d