#pragma once

#include "engine/physics/physics_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
emplace_or_replace_physics_world(entt::registry& r);

entt::entity
get_fixture(entt::registry& r, entt::entity e);

entt::entity
get_fixture_by_tag(entt::registry& r, entt::entity e, std::string tag);

std::vector<entt::entity>
get_all_in_area(entt::registry& r, glm::vec2 center, float d);

std::vector<std::pair<int, entt::entity>>
get_all_in_area_filtered(entt::registry& r,
                         const glm::vec2 center,
                         const float d,
                         const std::function<bool(entt::registry&, entt::entity)>& cond);

} // namespace game2d