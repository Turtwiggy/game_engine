#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
play_sound_if_hovered(entt::registry& r, std::vector<std::string>& hovered_buttons, const std::string& label);

void
ui_mute_sound_icon(entt::registry& r);

} // namespace game2d