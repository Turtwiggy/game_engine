#pragma once

// other libs
#include <glm/glm.hpp>

// std libs
#include <tuple>
#include <vector>

namespace game2d {

struct SINGLETON_LightingComponent
{
  std::vector<glm::ivec2> lights;
  std::vector<std::tuple<float, float, float>> intersections;
};

};