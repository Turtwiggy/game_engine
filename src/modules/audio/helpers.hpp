#pragma once

#include "game/components/app.hpp"
#include "resources/audio.hpp"

namespace game2d {

void
set_music_pause(bool on);

void
play_music(GameEditor& editor, const AvailableMusic& type);

void
stop_music(GameEditor& editor, const AvailableMusic& type);

void
play_sfx(GameEditor& editor, const AvailableSfx& type);

void
stop_sfx(GameEditor& editor, const AvailableSfx& type);

} // namespace game2d