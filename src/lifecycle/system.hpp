#pragma once

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_lifecycle_system(SINGLETON_EntityBinComponent& dead, entt::registry& r, const uint64_t& milliseconds_dt);

} // namespace game2d