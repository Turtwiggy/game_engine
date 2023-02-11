#pragma once

#include "AL/al.h"

#include "components.hpp"
#include "helpers.hpp"

namespace game2d {

ALuint
get_sound_id(const SINGLETON_AudioComponent& audio, const std::string& tag);

} // namespace game2d