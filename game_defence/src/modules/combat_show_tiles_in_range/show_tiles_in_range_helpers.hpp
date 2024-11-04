#pragma once

#include "modules/actor_player/components.hpp"
#include "modules/combat_show_tiles_in_range/show_tiles_in_range_components.hpp"
#include "modules/map/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_tiles_component(entt::registry& r,
                       entt::entity e,
                       const MapComponent& map_c,
                       const InputComponent& input_c,
                       const RangeType& t,
                       TilesComponent& c);

} // namespace game2d