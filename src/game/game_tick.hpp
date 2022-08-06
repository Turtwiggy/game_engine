#pragma once

#include "modules/events/components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

void
simulate(entt::registry& r, const std::vector<InputEvent>& inputs, uint64_t milliseconds_dt);

} // namespace game2d