#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <utility>
#include <unordered_map>

namespace game2d {

struct SINGLETON_TilemapComponent
{
  std::map<std::pair<int, int>, entt::entity> tilemap;
};

} // namespace game2d