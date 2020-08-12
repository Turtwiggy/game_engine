#pragma once

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

namespace fightingengine {

    class Transform3D
    {
    public:
        glm::vec3 Position = glm::vec3{ 0., 0., 0. };
        glm::vec3 Scale = glm::vec3{ 1., 1., 1. };
    };

}
