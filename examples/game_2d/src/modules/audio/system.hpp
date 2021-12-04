#pragma once

// engine headers
#include "engine/application.hpp"

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
init_audio_system(entt::registry& registry);

void
update_audio_system(entt::registry& registry, engine::Application& app, float dt);

} // namespace game2d