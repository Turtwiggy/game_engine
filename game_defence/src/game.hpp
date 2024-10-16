#pragma once

#include "engine/app/application.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
init(engine::SINGLE_Application& app, entt::registry& r);

void
init_slow(engine::SINGLE_Application& app, entt::registry& r);

void
fixed_update(engine::SINGLE_Application& app, entt::registry& r, const uint64_t milliseconds_dt);

void
update(engine::SINGLE_Application& app, entt::registry& r, const uint64_t milliseconds_dt);

} // namespace game2d