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
play_music(GameEditor& editor, const AvailableMusic& type)
{
  auto& audio = editor.audio;
  auto& res = audio.music;
  auto it = std::find_if(res.begin(), res.end(), [&type](const auto& a) { return a.type == type; });

  Mix_Music* data = it->data;
}

void
stop_music(GameEditor& editor, const AvailableMusic& type)
{
  auto& audio = editor.audio;
}

void
play_sfx(GameEditor& editor, const AvailableSfx& type)
{
  auto& audio = editor.audio;
}

void
stop_sfx(GameEditor& editor, const AvailableSfx& type)
{
  auto& audio = editor.audio;
}

} // namespace game2d