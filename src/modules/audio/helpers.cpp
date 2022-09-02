#include "helpers.hpp"

#include "resources/audio.hpp"

#include <SDL2/SDL_mixer.h>

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
play_music(entt::registry& r, std::string name)
{
  auto& audio = r.ctx().at<SINGLETON_AudioComponent>();
}

void
stop_music(entt::registry& r)
{
  auto& audio = r.ctx().at<SINGLETON_AudioComponent>();
}

void
play_sfx(entt::registry& r, std::string name)
{
  auto& audio = r.ctx().at<SINGLETON_AudioComponent>();
}

void
stop_sfx(entt::registry& r, std::string name)
{
  auto& audio = r.ctx().at<SINGLETON_AudioComponent>();
}

} // namespace game2d