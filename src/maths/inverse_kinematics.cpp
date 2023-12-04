#include "inverse_kinematics.hpp"

#include "maths/maths.hpp"

namespace game2d {

void
IKSolver::set_points_and_length(int points, float length)
{
  std::vector<glm::vec2> p;
  p.resize(points);
  lengths.resize(points);
  total_lengths = 0;

  for (int i = 0; i < points; i++) {
    p[i] = { 0, 0 }; // doesnt matter
    lengths[i] = length;
    total_lengths += length;
  }
}

std::vector<glm::vec2>
IKSolver::generate_backward_chain(const std::vector<glm::vec2>& points, const glm::vec2& goal) const
{
  std::vector<glm::vec2> results;
  results.resize(points.size());

  for (int i = points.size() - 1; i >= 0; i--) {

    // set last point to goal
    if (i == points.size() - 1) {
      results[i] = goal;
      continue;
    }

    // calculate the vector to the previous point
    const auto dir = glm::normalize(points[i] - results[i + 1]);

    const auto new_p = results[i + 1] + (dir * lengths[i]);
    results[i] = new_p;
  }

  return results;
};

std::vector<glm::vec2>
IKSolver::generate_forward_chain(const std::vector<glm::vec2>& points, const std::vector<glm::vec2>& back_chain) const
{
  std::vector<glm::vec2> results;
  results.resize(back_chain.size());

  for (int i = 0; i < back_chain.size(); i++) {

    // set the first point to the start
    if (i == 0) {
      results[0] = points[0];
      continue;
    }

    // calculate the vector to the next point
    const auto dir = glm::normalize(back_chain[i] - results[i - 1]);

    // TODO: Constraint: clamp between desired angles for this joint
    // const float constraint = 30.0f * engine::Deg2Rad;
    // dir.x = glm::clamp(dir.x, -1.0f, 1.0f);
    // dir.y = glm::clamp(dir.y, -1.0f, 1.0f);

    const auto new_p = results[i - 1] + (dir * lengths[i - 1]);
    results[i] = new_p;
  }

  return results;
};

std::vector<glm::vec2>
IKSolver::Iterate(const std::vector<glm::vec2>& in_points, const glm::vec2& goal)
{
  std::vector<glm::vec2> points = in_points;

  // Generate some points
  if (lengths.size() > 0 && in_points.size() == 0) {
    points.resize(lengths.size());
    for (int i = 0; i < lengths.size(); i++)
      points[i] = { i * lengths[i], i * lengths[i] };
  }

  if (points.size() == 0)
    return {};

  // If > than total distance, just max everything out
  const float total_distance = glm::distance(points[0], goal);
  if (total_distance > total_lengths) {
    std::vector<glm::vec2> ps;
    ps.resize(points.size());
    const auto dir = goal - points[0];
    for (int i = 1; i < points.size(); i++)
      ps[i] = points[i - 1] + glm::normalize(dir) * lengths[i - 1];
    return ps;
  }

  const auto bwd_chain = generate_backward_chain(points, goal);
  const auto fwd_chain = generate_forward_chain(points, bwd_chain);
  return fwd_chain; // updated ik
};
}