#pragma once

#include "components.hpp"
#include "helpers.hpp"

#include <AL/al.h>
#include <entt/entt.hpp>

#include <optional>
#include <string>

namespace game2d {

Sound
get_sound(const SINGLETON_AudioComponent& audio, const std::string& tag);

// https://github.com/kcat/openal-soft/blob/master/examples/common/alhelpers.c

void
close_audio(entt::registry& r);

void
open_audio_new_device(entt::registry& r, const std::optional<std::string>& name);

void
process_audio_added(entt::registry& r);

void
process_audio_removed(entt::registry& r);

void
stop_all_audio(entt::registry& r);

} // namespace game2d