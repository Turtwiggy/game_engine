#pragma once

#include "entt/entt.hpp"

namespace game2d {

void
init_audio_system(entt::registry& r);

void
update_audio_system(entt::registry& r, const float dt);

} // namespace game2d