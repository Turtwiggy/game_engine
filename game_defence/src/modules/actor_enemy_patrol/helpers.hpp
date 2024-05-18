#pragma once

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_patrol_from_desc(entt::registry& r, const entt::entity& e, PatrolDescription& desc);

} // namespace game2d