#pragma once

#include "trait_components.hpp"

#include <entt/entt.hpp>

#include <unordered_set>

namespace game2d {

bool
has_trait(entt::registry& r, const std::unordered_set<AquirableTrait>& traits, const AquirableTrait& t);

bool
has_trait(entt::registry& r, const std::vector<AquirableTrait>& traits, const AquirableTrait& t);

} // namespace game2d