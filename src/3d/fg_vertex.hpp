#pragma once

#include "glm/glm.hpp"

namespace fightinggame {

    struct FGVertex {
        glm::vec3 Position;
        glm::vec3 Normal = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec2 TexCoords = glm::vec2(0.0f, 0.0f);
        //glm::vec3 Tangent;
        //glm::vec3 Bitangent;
    };

}
