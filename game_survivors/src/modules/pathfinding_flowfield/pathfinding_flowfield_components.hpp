#pragma once

#include <entt/fwd.hpp>
#include <optional>

namespace game2d {

struct SINGLE_Flowfield
{
  std::map<int, int> came_from;
};

struct GenerateFlowfieldParams
{
  int xmax = 10;
  int ymax = 10;
  int tilesize = 32;
  int from_idx = 0;
};

struct RequestGenerateFlowField
{
  std::optional<GenerateFlowfieldParams> params = std::nullopt;
};

} // namespace game2d