#pragma once

#include "engine/app/application.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
init_input_system(entt::registry& r);

void
update_input_system(engine::SINGLE_Application& app, entt::registry& r);

} // namespace game2d