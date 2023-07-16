#pragma once

#include "app/application.hpp"
#include "entt/entt.hpp"

namespace game2d {

void
init(engine::SINGLETON_Application& app, entt::registry& game);

void
fixed_update(entt::registry& game, uint64_t milliseconds_dt);

void
update(engine::SINGLETON_Application& app, entt::registry& game, float dt);

} // namespace game2d