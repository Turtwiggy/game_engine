#pragma once

#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"

namespace game2d {

EntityType
generate_item(const int floor, engine::RandomState& rnd);

EntityType
generate_monster(const int floor, engine::RandomState& rnd);

} // namespace game2d