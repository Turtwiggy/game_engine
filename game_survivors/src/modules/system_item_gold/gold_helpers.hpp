#pragma once

#include "gold_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

SINGLE_GoldComponent
load_gold_from_disk(entt::registry& r);

} // namespace game2d