#pragma once

#include <glm/glm.hpp>

namespace fightingengine {

    struct ColourVec4f {
        glm::vec4 colour = glm::vec4(1.0f, (110/255.0f), (199/255.0f), 1.0f);
    };

    ///thanks to https://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html for the approximations
    template<int N, typename T>
    inline
        constexpr glm::vec<N, T> srgb_to_lin_approx(const glm::vec<N, T>& in)
    {
        return
            0.012522878f * in +
            0.682171111f * in * in +
            0.305306011f * in * in * in;
    }

    template<int N, typename T>
    inline
        glm::vec<N, T> lin_to_srgb(const glm::vec<N, T>& in)
    {
        auto ret = in;

        for (int i = 0; i < N; i++)
        {
            if (in.v[i] <= 0.0031308)
                ret.v[i] = in.v[i] * 12.92;
            else
                ret.v[i] = 1.055 * pow(in.v[i], 1.0 / 2.4) - 0.055;
        }

        return ret;
    }

    template<int N, typename T>
    inline
        constexpr glm::vec<N, T> lin_to_srgb_approx(const glm::vec<N, T>& in)
    {
        vec<N, T> S1 = sqrtf(in);
        vec<N, T> S2 = sqrtf(S1);
        vec<N, T> S3 = sqrtf(S2);

        return 0.662002687f * S1 + 0.684122060f * S2 - 0.323583601f * S3 - 0.0225411470f * in;
    }

}
