#pragma once

#include "modules/gen_dungeons/components.hpp"

namespace game2d {

bool
collide(const RoomAABB& a, const RoomAABB& b);

} // namespace game2d