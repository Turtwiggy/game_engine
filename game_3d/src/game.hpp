#pragma once

#include "app/application.hpp"
#include "entt/entt.hpp"

namespace game3d {

void
init(engine::SINGLETON_Application& app, entt::registry& r);

void
fixed_update(entt::registry& r, const uint64_t milliseconds_dt);

void
update(engine::SINGLETON_Application& app, entt::registry& r, const float dt);

} // namespace game2d