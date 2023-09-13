#pragma once

#include "app/application.hpp"
#include "cli.hpp"

#include "entt/entt.hpp"

namespace game2d {

void
init(engine::SINGLETON_Application& app, entt::registry& r, const Cli& cli);

void
fixed_update(entt::registry& r, const uint64_t milliseconds_dt);

void
update(engine::SINGLETON_Application& app, entt::registry& r, const Cli& cli, const float dt);

} // namespace