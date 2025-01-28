#pragma once

#include "trait_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

bool
has_trait(entt::registry& r, const std::vector<AquirableTrait>& traits, const AquirableTrait& t);

} // namespace game2d