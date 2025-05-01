#pragma once

namespace game2d {

// returns noise ~[-1, 1]
float
perlin_noise_2d(float x, float y, int seed = 0);

} // namespace game2d