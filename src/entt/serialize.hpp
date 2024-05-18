#pragma once

#include <entt/entt.hpp>

#include <string>

namespace game2d {

void
save(const entt::registry& registry, const std::string& s);

void
load(entt::registry& r, const std::string& s);

void
load_if_exists(entt::registry& registry, const std::string& s);

} // namespace game2d