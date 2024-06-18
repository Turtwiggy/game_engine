#include "cell_automata.hpp"

#include "maths/grid.hpp"
#include "maths/maths.hpp"

#include <utility>

namespace game2d {

std::vector<int>
generate_50_50(const glm::ivec2 xy, const int seed)
{
  engine::RandomState rnd(seed);

  std::vector<int> results;

  for (int i = 0; i < xy.x * xy.y; i++) {
    int res = engine::rand_01(rnd.rng) >= 0.5f ? 1 : 0;
    results.push_back(res);
  }

  return results;
};

std::vector<int>
iterate_with_cell_automata(const std::vector<int>& in, const glm::ivec2& xy)
{
  assert(in.size() == xy.x * xy.y);

  std::vector<int> results;

  for (int i = 0; i < xy.x * xy.y; i++) {
    const auto grid_xy = engine::grid::index_to_grid_position(i, xy.x, xy.y);
    const auto neighbours_idxs = engine::grid::get_neighbour_indicies_with_diagonals(grid_xy.x, grid_xy.y, xy.x, xy.y);

    int neighbours = 0;

    for (const auto [dir, idx] : neighbours_idxs) {
      const auto val = in[idx];
      neighbours += val;
    }

    // Cell automata rules

    int new_val = 0;
    if (neighbours > 4 || neighbours == 0)
      new_val = 1;
    else
      new_val = 0;

    results.push_back(new_val);
  }

  return results;
}

}