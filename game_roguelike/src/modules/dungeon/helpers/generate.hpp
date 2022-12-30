#pragma once

#include "maths/maths.hpp"
#include "components/actors.hpp"

namespace game2d {

EntityType
generate_item(const int floor, engine::RandomState& rnd);

EntityType
generate_monster(const int floor, engine::RandomState& rnd);

} // namespace game2d