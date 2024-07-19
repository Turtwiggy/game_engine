#pragma once

#include "physics/components.hpp"
#include "physics/helpers.hpp"
#include <glm/glm.hpp>
#include <maths/grid.hpp>
#include <maths/maths.hpp>

#include <optional>

namespace game2d {

std::vector<glm::vec2> inline generate_poisson(const int w, const int h, const float radius, const int seed = 0)
{
  // setup
  const AABB rect = { { w / 2.0f, h / 2.0f }, { w, h } };
  const auto r2 = radius * radius;
  const float grid_size = radius / glm::sqrt(2.0f);
  const auto grid_w = static_cast<int>(glm::ceil(w / grid_size));
  const auto grid_h = static_cast<int>(glm::ceil(h / grid_size));
  auto rnd = engine::RandomState();
  rnd.rng.seed(seed);
  const auto k = 30; // max attempts bfore making sample inactive

  std::vector<std::optional<glm::vec2>> results(grid_w * grid_h, std::nullopt);
  std::vector<glm::vec2> active_samples;

  // lambdas
  const auto add_sample = [&active_samples, &results, &grid_size, &grid_w, &r2](const glm::vec2& sample) {
    active_samples.push_back(sample);

    const auto grid_pos = engine::grid::world_space_to_grid_space(sample, grid_size);
    const auto grid_idx = engine::grid::grid_position_to_index(grid_pos, grid_w);
    results[grid_idx] = sample;
  };
  const auto is_far_enough = [&results, &grid_size, &grid_w, &grid_h, &r2](const glm::vec2& sample) {
    const auto pos = engine::grid::world_space_to_grid_space(sample, grid_size);

    const int xmin = glm::max(pos.x - 2, 0);
    const int ymin = glm::max(pos.y - 2, 0);
    const int xmax = glm::min(pos.x + 2, grid_w - 1);
    const int ymax = glm::min(pos.y + 2, grid_h - 1);

    for (int y = ymin; y <= ymax; y++) {
      for (int x = xmin; x <= xmax; x++) {
        const auto idx = engine::grid::grid_position_to_index({ x, y }, grid_w);
        const auto s = results[idx];
        if (s != std::nullopt) {
          const glm::vec2 d = { s.value().x - sample.x, s.value().y - sample.y };
          if (d.x * d.x + d.y * d.y < r2)
            return false;
        }
      }
    }
    return true;
  };

  // first random sample
  const auto x = engine::rand_01(rnd.rng) * rect.size.x;
  const auto y = engine::rand_01(rnd.rng) * rect.size.y;
  add_sample({ x, y });

  // generate samples
  while (active_samples.size() > 0) {
    // Pick a random active sample
    const int i = static_cast<int>(engine::rand_01(rnd.rng) * active_samples.size());
    const auto& sample = active_samples[i];

    // Try "k" random candidates between [radius, 2 * radius] from that sample
    bool found = false;
    for (int j = 0; j < k; ++j) {
      const float angle = 2.0f * engine::PI * engine::rand_01(rnd.rng);

      // See: http://stackoverflow.com/questions/9048095/create-random-number-within-an-annulus/9048443#9048443
      const float r = glm::sqrt(engine::rand_01(rnd.rng) * 3.0f * r2 + r2);
      const glm::vec2 candidate = sample + (r * glm::vec2{ glm::cos(angle), glm::sin(angle) });

      // Accept candidates if it's inside the rect and further than 2 * radius to any existing sample
      if (contains<glm::vec2>(rect, candidate) && is_far_enough(candidate)) {
        found = true;
        add_sample(candidate);
        break;
      }
    }

    if (!found)
      active_samples.erase(active_samples.begin() + i);
  }

  // Filter out all the nullopts
  std::vector<glm::vec2> filtered_results;
  for (int i = 0; i < results.size(); i++) {
    if (results[i] != std::nullopt)
      filtered_results.push_back(results[i].value());
  }
  return filtered_results;
}
} // namespace game2d