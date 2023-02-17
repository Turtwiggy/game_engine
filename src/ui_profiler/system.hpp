#pragma once

#include "physics/components.hpp"
#include "ui_profiler/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_ui_profiler_system(Profiler& profiler, const SINGLETON_PhysicsComponent& physics, entt::registry& r);

} // namespace game2d