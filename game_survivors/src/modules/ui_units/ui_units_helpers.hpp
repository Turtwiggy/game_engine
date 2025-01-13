#pragma once

#include "ui_units_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

std::vector<UnitType>
load_units(entt::registry& r);

// Could improve: no need to save the entire
// std::vector<UnitType> only the units that have changed...
void
save_units(entt::registry& r, const std::vector<UnitType>& units);

void
add_unit_to_entt(entt::registry& r, const UnitType& unit);

} // namespace game2d