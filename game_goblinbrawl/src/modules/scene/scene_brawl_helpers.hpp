#pragma once

#include "modules/actor_brawler_spawner/actor_brawler_spawner_components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

// Create 4 walls for the cage
void
create_walls(entt::registry& r);

entt::entity
create_brawler(entt::registry& r, const BrawlerSpawner& spawner, const glm::vec2 pos);

} // namespace game2d