#include "pch.hpp"

#include "poisson.hpp"

#include "engine/maths/aabb.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"

namespace game2d {

PoissonOut
generate_poisson(const PoissonIn& in)
{
  static engine::RandomState rnd;
  rnd.rng.seed(in.seed);

  const auto aabb = AABB{ in.tl, in.wh };
  const auto r2 = in.radius * in.radius;
  const float grid_size = 2.0f * in.radius;
  const int grid_w = static_cast<int>(in.wh.x / grid_size);
  const int grid_h = static_cast<int>(in.wh.y / grid_size);

  std::unordered_map<int, std::vector<glm::vec2>> results;
  std::vector<glm::vec2> active_samples;

  const auto add_sample = [&](const glm::vec2 sample) {
    active_samples.push_back(sample);

    const auto grid_pos = engine::grid::worldspace_to_gridspace(sample, grid_size);
    const auto grid_idx = engine::grid::grid_position_to_index(grid_pos, grid_w);
    auto& vec = results.try_emplace(grid_idx).first->second;
    vec.push_back(sample);
  };
  const auto is_far_enough = [&](const glm::vec2 sample) {
    const auto grid_pos = engine::grid::worldspace_to_gridspace(sample, grid_size);

    const int xmin = grid_pos.x - 2;
    const int ymin = grid_pos.y - 2;
    const int xmax = grid_pos.x + 2;
    const int ymax = grid_pos.y + 2;

    for (int y = ymin; y <= ymax; y++) {
      for (int x = xmin; x <= xmax; x++) {
        const auto grid_idx = engine::grid::grid_position_to_index({ x, y }, grid_w);
        const auto other_samples = results[grid_idx];
        for (auto other_sample : other_samples) {
          const auto d = (other_sample - sample);
          const auto d2 = d.x * d.x + d.y * d.y;
          if (d2 <= r2)
            return false; // point is too close to another point
        }
      }
    }

    return true;
  };

  // first random sample.
  const auto x = engine::rand_01(rnd.rng) * in.wh.x;
  const auto y = engine::rand_01(rnd.rng) * in.wh.y;
  add_sample({ x, y });

  while (active_samples.size() > 0) {
    // Pick a random active sample
    const auto sample = active_samples[(int)(engine::rand_01(rnd.rng) * active_samples.size())];

    // try n random candidates between [radius, 2 * radius]
    bool found = false;
    for (int i = 0; i < in.attempts; i++) {
      const float angle = engine::TWO_PI * engine::rand_01(rnd.rng);

      // See: http://stackoverflow.com/questions/9048095/create-random-number-within-an-annulus/9048443#9048443
      const auto r = glm::sqrt(engine::rand_01(rnd.rng) * 3.0f * r2 + r2);
      const auto candidate = sample + (r * glm::vec2{ glm::cos(angle), glm::sin(angle) });

      // Accept candidates if it's inside the rect and further than 2 * radius to any existing sample
      if (contains(aabb, candidate) && is_far_enough(candidate)) {
        found = true;
        add_sample(candidate);
        break;
      }
    }

    if (!found)
      std::erase(active_samples, sample);
  }

  // flatten the std::unordered_set<std::vector<>> to std::vector<>
  PoissonOut out;
  const auto vectors = results | std::views::values;
  const auto flattened = vectors | std::views::join;
  out.results = { flattened.begin(), flattened.end() };
  return out;
};

} // namespace game2d