#pragma once

namespace game2d {

struct IslandNearestComponent
{
  // island_e <=> pos
  std::vector<std::pair<entt::entity, glm::vec2>> landable_positions;
};

} // namespace game2d