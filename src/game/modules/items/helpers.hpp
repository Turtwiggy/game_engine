#pragma once

#include "game/components/app.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
use_item(GameEditor& editor, Game& game, const entt::entity& user, const entt::entity& item);

} // namespace game2d