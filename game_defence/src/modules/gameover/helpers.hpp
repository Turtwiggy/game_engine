#pragma once

#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace game2d {

void
restart_game(entt::registry& r, b2World& world);

} // namespace game2d