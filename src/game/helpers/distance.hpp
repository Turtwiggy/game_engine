#pragma once

#include "game/components/app.hpp"

#include <entt/entt.hpp>

namespace game2d {

entt::entity
get_nearest_attackable(Game& g, const entt::entity& e);

} // namespace game2d