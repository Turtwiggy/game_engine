#pragma once

#include <entt/entt.hpp>

#include <string>

namespace game2d {

void
set_music_pause(bool on);

void
play_music(entt::registry& r, std::string name);

void
stop_music(entt::registry& r);

void
play_sfx(entt::registry& r, std::string name);

void
stop_sfx(entt::registry& r, std::string name);

} // namespace game2d