#pragma once

#include "shop_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

SINGLE_Shop
load_shop(entt::registry& r, std::string path);

} // namespace game2d