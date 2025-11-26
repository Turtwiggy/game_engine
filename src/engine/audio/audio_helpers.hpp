#pragma once

#include "audio_components.hpp"

#include <string>

namespace game2d {

Sound
get_sound(const SINGLE_AudioComponent& audio, const std::string& tag);

bool
refresh_audio(entt::registry& r, std::string device_name = "");

} // namespace game2d