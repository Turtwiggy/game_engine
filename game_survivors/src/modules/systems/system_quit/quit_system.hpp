#pragma once

#include "engine/app/application.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
update_quit_system(entt::registry& r, engine::SINGLE_Application& app);

} // namespace game2d