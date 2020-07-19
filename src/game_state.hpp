#pragma once

#include "3d/fg_object.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <thread>

namespace fightinggame {

    struct GameState
    {
    public:

        GameState(std::shared_ptr<FGObject> transform)
            : cube(transform)
        {}

        std::shared_ptr<FGObject> cube;
    };

}
