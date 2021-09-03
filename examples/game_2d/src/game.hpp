#pragma once

// c++ headers
#include <chrono>

// other project headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "components/resources.hpp"

namespace game2d {

namespace game {

void
init(entt::registry& registry, glm::ivec2 screen_wh, std::chrono::steady_clock::time_point app_start);

void
update_sim(entt::registry& registry, float dt);

void
update(entt::registry& registry, float dt);

void
render(entt::registry& registry);

}; // namespace game

}; // namespace game2d