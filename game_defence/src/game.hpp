#pragma once

#include "app/application.hpp"
#include "box2d/box2d.h"
#include "entt/entt.hpp"

namespace game2d {

void
init(engine::SINGLETON_Application& app, b2World& world, entt::registry& game);

void
fixed_update(entt::registry& game, b2World& world, const uint64_t milliseconds_dt);

void
update(engine::SINGLETON_Application& app, entt::registry& game, const float dt);

} // namespace game2d