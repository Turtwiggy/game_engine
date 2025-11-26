#pragma once

#include "entt/entity/fwd.hpp"
#include "hulls_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

SINGLE_Hulls
load_hulls(entt::registry& r, std::string directory);

} // namespace game2d