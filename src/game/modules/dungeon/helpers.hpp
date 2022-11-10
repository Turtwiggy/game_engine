#pragma once

#include "components.hpp"
#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
#include "game/components/app.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace game2d {

void
transfer_old_state_generate_dungeon(GameEditor& editor, Game& game, uint32_t& seed);

} // namespace game2d