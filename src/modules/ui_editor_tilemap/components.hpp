#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <unordered_map>
#include <utility>

namespace game2d {

struct SINGLETON_TilemapComponent
{
  std::map<std::pair<int, int>, entt::entity> tilemap;
};

} // namespace game2d