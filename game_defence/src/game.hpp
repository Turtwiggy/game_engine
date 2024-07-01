#pragma once

#include "app/application.hpp"
#include "entt/entt.hpp"

namespace game2d {

void
init(engine::SINGLETON_Application& app, entt::registry& game);

void
init_slow(engine::SINGLETON_Application& app, entt::registry& r);

void
fixed_update(engine::SINGLETON_Application& app, entt::registry& game, const uint64_t milliseconds_dt);

void
update(engine::SINGLETON_Application& app, entt::registry& game, const float dt);

} // namespace game2d