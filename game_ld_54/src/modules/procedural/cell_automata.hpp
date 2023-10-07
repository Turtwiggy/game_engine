#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace game2d {

// make the map complete chaos, 50% 0s, 50% 1s
std::vector<int>
generate_50_50(const glm::ivec2 xy, const int seed);

// iteratively apply the cellular automata rules
std::vector<int>
iterate_with_cell_automata(const std::vector<int>& in, const glm::ivec2& xy);

// remove any isolated obstacles
// void
// generation_post_processing()
// {
// }

// start at the middle, and walk left until open tile
// void
// generate_start_point()
// {
// }

// floodfill all the tiles we can reach from the starting point
// get the tile furthest away
// void
// generate_end_point()
// {
// }

} // namespace game2d