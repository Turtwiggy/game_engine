#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <unordered_map>
#include <utility>

namespace game2d {

struct TilemapComponent
{
  std::map<std::pair<int, int>, entt::entity> tilemap;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TilemapComponent, tilemap);
};

} // namespace game2d