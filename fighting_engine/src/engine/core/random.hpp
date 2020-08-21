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

///https://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
template<int N, typename T>
inline
vec<N, T> srgb_to_lin(const vec<N, T>& in)
{
    auto ret = in;

    for (int i = 0; i < N; i++)
    {
        if (in.v[i] < 0.04045)
            ret.v[i] = in.v[i] / 12.92;
        else
            ret.v[i] = pow((in.v[i] + 0.055) / 1.055, 2.4);
    }

    return ret;
}
