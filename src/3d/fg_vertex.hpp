#pragma once

#include "glm/glm.hpp"

namespace fightinggame {

    struct FGVertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

}
