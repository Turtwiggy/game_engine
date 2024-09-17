// #include "poisson.hpp"

// #include "engine/physics/components.hpp"
// #include "engine/physics/helpers.hpp"
// #include <maths/grid.hpp>
// #include <maths/maths.hpp>

// namespace game2d {

// std::vector<glm::vec2>
// generate_poisson(const int w, const int h, const float radius, const int seed)
// {
//   // setup
//   const auto k = 30; // max attempts bfore making sample inactive
//   const AABB rect = { { w / 2, h / 2 }, { w, h } };
//   const auto r2 = radius * radius;
//   const float cell_size = radius / glm::sqrt(2.0f);

//   const int grid_w = glm::ceil(w / cell_size);
//   const int grid_h = glm::ceil(h / cell_size);
//   std::vector<glm::vec2> grid(grid_w * grid_h, glm::vec2(0.0f, 0.0f));

//   std::vector<glm::vec2> active_samples;

//   const auto add_sample = [&](const glm::vec2& sample) {
//     active_samples.push_back(sample);
//     const auto grid_pos = engine::grid::worldspace_to_grid_space(sample, cell_size);
//     const auto grid_idx = engine::grid::grid_position_to_index(grid_pos, grid_w);
//     grid[grid_idx] = sample;
//   };

//   // first random sample
//   auto rnd = engine::RandomState(seed);
//   const int x = engine::rand_01(rnd.rng) * rect.size.x;
//   const int y = engine::rand_01(rnd.rng) * rect.size.y;
//   add_sample({ x, y });

//   const auto is_far_enough = [&grid, &cell_size, &grid_w, &grid_h, &r2](const glm::vec2& sample) {
//     const auto pos = engine::grid::worldspace_to_grid_space(sample, cell_size);

//     const int xmin = glm::max(pos.x - 2, 0);
//     const int ymin = glm::max(pos.y - 2, 0);
//     const int xmax = glm::min(pos.x + 2, grid_w - 1);
//     const int ymax = glm::min(pos.y + 2, grid_h - 1);

//     for (int y = ymin; y <= ymax; y++) {
//       for (int x = xmin; x <= xmax; x++) {
//         const auto idx = engine::grid::grid_position_to_index({ x, y }, grid_w);
//         const auto s = grid[idx];
//         if (s != glm::vec2(0.0f, 0.0f)) {
//           const glm::vec2 d = s - sample;
//           if (d.x * d.x + d.y * d.y < r2)
//             return false;
//         }
//       }
//     }
//     return true;

//     // Note: we use the zero vector to denote an unfilled cell in the grid. This means that if we were
//     // to randomly pick (0, 0) as a sample, it would be ignored for the purposes of proximity-testing
//     // and we might end up with another sample too close from (0, 0). This is a very minor issue.
//   };

//   // generate samples
//   while (active_samples.size() > 0) {
//     // Pick a random active sample
//     const int i = static_cast<int>(engine::rand_01(rnd.rng) * active_samples.size());
//     const auto& sample = active_samples[i];

//     // Try "k" random candidates between [radius, 2 * radius] from that sample
//     bool found = false;
//     for (int j = 0; j < k; ++j) {
//       const float angle = 2.0f * engine::PI * engine::rand_01(rnd.rng);

//       // See: http://stackoverflow.com/questions/9048095/create-random-number-within-an-annulus/9048443#9048443
//       const float r = glm::sqrt(engine::rand_01(rnd.rng) * 3.0f * r2 + r2);
//       const glm::vec2 candidate = sample + (r * glm::vec2{ glm::cos(angle), glm::sin(angle) });

//       // Accept candidates if it's inside the rect and further than 2 * radius to any existing sample
//       if (contains<glm::vec2>(rect, candidate) && is_far_enough(candidate)) {
//         found = true;
//         add_sample(candidate);
//         break;
//       }
//     }

//     // If we couldn't find a valid candidate after k attempts, remove this sample from the active samples queue
//     if (!found) {
//       active_samples[i] = active_samples[active_samples.size() - 1];
//       active_samples.erase(active_samples.begin() + active_samples.size() - 1);
//     }
//   }

//   const int half_w = 0;
//   const int half_h = 0;

//   // Filter out all the nullopts
//   std::vector<glm::vec2> results;
//   for (int i = 0; i < grid.size(); i++) {
//     if (grid[i] != glm::vec2(0.0f, 0.0f))
//       results.push_back({ half_w + grid[i].x, half_h + grid[i].y });
//   }
//   return results;
// }

// } // namespace game2d