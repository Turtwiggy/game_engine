#pragma once

#include "game/components/app.hpp"
#include "game/modules/items/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

entt::entity
has_equipped(Game& game, const entt::entity& parent, const EquipmentSlot& slot);

} // namespace game2d