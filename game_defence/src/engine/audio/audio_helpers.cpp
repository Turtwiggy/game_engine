#include "audio_helpers.hpp"

#include <SDL2/SDL_log.h>
#include <algorithm>
#include <format>

namespace game2d {

Sound
get_sound(const SINGLE_AudioComponent& audio, const std::string& tag)
{
  const auto& res = audio.sounds;
  auto it = std::find_if(res.begin(), res.end(), [&tag](const auto& a) { return a.tag == tag; });
  return (*it);
};

} // namespace game2d