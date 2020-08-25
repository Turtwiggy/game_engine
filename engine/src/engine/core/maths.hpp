#pragma once

#include "engine/core/random.hpp"

#include <glm/glm.hpp>

double pi = 3.1415926535897932385;

float length_squared(const glm::vec3& i)
{
    return i.x * i.x + i.y * i.y + i.z * i.z;
}

glm::vec3 rand_unit_in_sphere(random_state& rnd, float M, float MN)
{
    auto a = rand_det_s(rnd.rng, 0, 2.0*pi);
    auto z = rand_det_s(rnd.rng, -1, 1);
    auto r = glm::sqrt(1 - z*z);

    return glm::vec3(r*glm::cos(a), r*glm::sin(a), z);
}


