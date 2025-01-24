#pragma once

#include "trait_components.hpp"

#include <entt/entt.hpp>

#include <string>

namespace game2d {

bool
has_trait(entt::registry& r, const std::vector<Trait>& traits, const std::string& key);

} // namespace game2d