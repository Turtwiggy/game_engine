#pragma once

// other library headers
#include <glm/glm.hpp>

// your library headers
#include "engine/maths/random.hpp"

namespace fightingengine {

constexpr float PI{ 3.1415926535897932385f };

float
length_squared(const glm::vec3& i);

glm::vec3
rand_unit_vector(RandomState& rnd);

glm::vec3
random_in_hemisphere(RandomState& rnd, glm::vec3& normal);

} // namespace fightingengine
