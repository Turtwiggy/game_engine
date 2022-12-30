#pragma once

#include "components/app.hpp"
#include "modules/items/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

entt::entity
has_equipped(Game& game, const entt::entity& parent, const EquipmentSlot& slot);

} // namespace game2d