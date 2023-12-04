#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace game2d {

struct IKSolver
{
  void set_points_and_length(int points, float length);
  std::vector<glm::vec2> Iterate(const std::vector<glm::vec2>& in_points, const glm::vec2& goal);

private:
  std::vector<float> lengths;
  float total_lengths = 0.0f;

  std::vector<glm::vec2> generate_backward_chain(const std::vector<glm::vec2>& points, const glm::vec2& goal) const;
  std::vector<glm::vec2> generate_forward_chain(const std::vector<glm::vec2>& points,
                                                const std::vector<glm::vec2>& back_chain) const;
};

} // namespace game2d