#pragma once

#include <entt/entt.hpp>

namespace game2d {

namespace audio {

namespace sdl_mixer {

std::vector<std::string>
list_devices();

void
stop_all_audio(entt::registry& r);

void
process_audio_added(entt::registry& r);

void
process_audio_removed(entt::registry& r);

} // namespace audio

} // namespace sdl_mixer

} // namespace game2d