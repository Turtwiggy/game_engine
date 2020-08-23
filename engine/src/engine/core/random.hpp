#pragma once

#include <random>



struct random_state
{
    std::minstd_rand rng;
};

//https://github.com/20k/funvectormaths/blob/master/vec.hpp
///falls into the pidgeonhole trap
///and also std::minstd_rand isn't a very good rng
///unfortunately the distributions in <random> are unspecified, so will not get the same results
///across platforms, making them useless for anything which needs reproducible random numbers, aka everything
template<typename T>
inline
float rand_det_s(T& rnd, float M, float MN)
{
    float scaled = (rnd() - rnd.min()) / (float)(rnd.max() - rnd.min() + 1.f);

    return scaled * (MN - M) + M;
}