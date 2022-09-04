#include "helpers.hpp"

#include "resources/audio.hpp"

#include <SDL2/SDL_mixer.h>

#include <algorithm>

namespace game2d {

void
set_music_pause(bool on)
{
  if (on)
    Mix_PauseMusic();
  else
    Mix_ResumeMusic();
};

void
play_music(entt::registry& r, const AvailableMusic& type)
{
  auto& res = r.ctx().at<SINGLETON_AudioComponent>().music;
  auto it = std::find_if(res.begin(), res.end(), [&type](const auto& a) { return a.type == type; });

  Mix_Music* data = it->data;
}

void
stop_music(entt::registry& r, const AvailableMusic& type)
{
  auto& audio = r.ctx().at<SINGLETON_AudioComponent>();
}

void
play_sfx(entt::registry& r, const AvailableSfx& type)
{
  auto& audio = r.ctx().at<SINGLETON_AudioComponent>();
}

void
stop_sfx(entt::registry& r, const AvailableSfx& type)
{
  auto& audio = r.ctx().at<SINGLETON_AudioComponent>();
}

} // namespace game2d