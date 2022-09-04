#include "audio.hpp"

#include <algorithm>

namespace game2d {

Music&
get_music(entt::registry& r, AvailableMusic type)
{
  auto& res = r.ctx().at<SINGLETON_AudioComponent>().music;
  auto it = std::find_if(res.begin(), res.end(), [&type](const auto& a) { return a.type == type; });
  return *it;
};

Sfx&
get_sfx(entt::registry& r, AvailableSfx type)
{
  auto& res = r.ctx().at<SINGLETON_AudioComponent>().sfx;
  auto it = std::find_if(res.begin(), res.end(), [&type](const auto& a) { return a.type == type; });
  return *it;
};

} // namespace game2d