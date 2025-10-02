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

struct IslandShoreTriangle
{
  bool placeholder = true;
};

struct RequestGenerateRocks
{
  bool placeholder = true;
};

struct NoiseInfo
{
  float noise = 0.0f;
  glm::ivec2 xy = { 0, 0 };
};

struct DebugContoursComponent
{
  std::vector<Edge> edges;
  std::vector<Edge> sorted_edges;
  std::vector<NoiseInfo> island_noise; // note: xy is not offset in island_noise

  std::vector<glm::ivec2> all_island_xy;
  std::vector<std::pair<glm::ivec2, entt::entity>> occupied_island_xy;
};

struct SINGLE_Islands
{
  const int map_size = 4096;
  const int tilesize = 32;
  const int wh = map_size / tilesize;
  float cutoff = 0.69f; // [0, 1]
  float frequency = 0.1f;

  // the noise that generated all the islands
  std::vector<NoiseInfo> generated;

  // the grid id (stored via cantor pairing function) <=> island entity map
  std::unordered_map<uint64_t, entt::entity> id_to_island_eid;

  static SINGLE_Islands instance;
};

} // namespace game2d