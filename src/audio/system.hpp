#pragma once

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
init_audio_system(entt::registry& r, SINGLETON_AudioComponent& audio);

void
update_audio_system(entt::registry& r, SINGLETON_AudioComponent& audio);

} // namespace game2d