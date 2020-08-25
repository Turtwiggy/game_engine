#pragma once

#include "engine/core/random.hpp"

#include <glm/glm.hpp>

double pi = 3.1415926535897932385;

float length_squared(const glm::vec3& i)
{
    return i.x * i.x + i.y * i.y + i.z * i.z;
}

// glm::vec3 rand_unit_vector(random_state& rnd, float min = 0, float max = 1)
// {
//     return glm::vec3(
//         rand_det_s(rnd.rng, min, max),
//         rand_det_s(rnd.rng, min, max),
//         rand_det_s(rnd.rng, min, max)
//     );
// }

glm::vec3 rand_unit_vector(random_state& rnd)
{
    auto a = rand_det_s(rnd.rng, 0, 2.0*pi);
    auto z = rand_det_s(rnd.rng, -1, 1);
    auto r = glm::sqrt(1 - z*z);
    return glm::vec3(r*glm::cos(a), r*glm::sin(a), z);
}

// glm::vec3 rand_in_unit_sphere(random_state& rnd) {
//     while (true) {
//         glm::vec3 p = rand_unit_vector(rnd);
//         if (length_squared(p) >= 1) continue;
//         return p;
//     }
// }

glm::vec3 random_in_hemisphere(random_state& rnd, glm::vec3& normal) {
    glm::vec3 in_unit_sphere = rand_unit_vector(rnd);
    if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}


