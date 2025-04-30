#pragma once

namespace game2d {

struct PoissonIn
{
  int seed = 0;
  int attempts = 10; // before random pos is considered invalid

  float radius = 50.0f; // space between points
  glm::vec2 tl{ 0, 0 };
  glm::ivec2 wh{ 100, 100 };
};

struct PoissonOut
{
  std::vector<glm::vec2> results;
};

PoissonOut
generate_poisson(const PoissonIn& in);

} // namespace game2d