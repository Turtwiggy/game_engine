#pragma once

#include "glm/glm.hpp"

namespace fightinggame {

    struct FGVertex {

        FGVertex()
            : Position(glm::vec3(0.0, 0.0, 0.0))
            , Normal(glm::vec3(0.0, 0.0, 0.0))
            , TexCoords(glm::vec2(0.0, 0.0))
        {};

        FGVertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 tex)
            :Position(pos)
            ,Normal(normal)
            ,TexCoords(tex)
        {
        }

        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        //glm::vec3 Tangent;
        //glm::vec3 Bitangent;
    };

}
