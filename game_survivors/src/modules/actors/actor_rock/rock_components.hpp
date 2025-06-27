#pragma once

#include "engine/map/components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

struct BoundingBoxComponent
{
  glm::vec2 tl{ FLT_MAX, FLT_MAX };
  glm::vec2 br{ -FLT_MAX, -FLT_MAX };
};

struct RockComponent
{
  bool placeholder = true;
};

struct RequestGenerateRocks
{
  bool placeholder = true;
};

struct DebugContoursComponent
{
  std::vector<Edge> edges;
  std::vector<Edge> sorted_edges;
};

struct NoiseInfo
{
  std::optional<float> noise = std::nullopt;
  glm::ivec2 xy = { 0, 0 };
};

struct SINGLE_Islands
{
  // the noise that generated all the islands
  std::vector<NoiseInfo> info;

  // the grid id (stored via cantor pairing function) <=> island entity map
  std::unordered_map<uint64_t, entt::entity> id_to_island_eid;

  static SINGLE_Islands instance;
};

} // namespace game2d