#pragma once

#include "resources/audio.hpp"

#include <entt/entt.hpp>

#include <string>

namespace game2d {

void
set_music_pause(bool on);

void
play_music(entt::registry& r, const AvailableMusic& type);

void
stop_music(entt::registry& r, const AvailableMusic& type);

void
play_sfx(entt::registry& r, const AvailableSfx& type);

void
stop_sfx(entt::registry& r, const AvailableSfx& type);

} // namespace game2d