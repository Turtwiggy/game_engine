#pragma once

#include "engine/core/random.hpp"

#include <glm/glm.hpp>

namespace fightingengine
{
    #define PI 3.1415926535897932385

    float length_squared(const glm::vec3 &i);

    glm::vec3 rand_unit_vector(RandomState &rnd);

    glm::vec3 random_in_hemisphere(RandomState &rnd, glm::vec3 &normal);

} // namespace fightingengine
