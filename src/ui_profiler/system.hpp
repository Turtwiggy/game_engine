#pragma once

#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace game2d {

void
update_ui_profiler_system(entt::registry& r, const b2World& world);

} // namespace game2d