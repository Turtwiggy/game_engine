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

        GameState(
            std::vector<std::shared_ptr<FGObject>> transforms,
            std::shared_ptr<FGObject> terrain,
            std::shared_ptr<FGObject> player,
            std::shared_ptr<FGObject> cornel_box )
            : cubes(transforms)
            , terrain(terrain)
            , player(player)
            , cornel_box(cornel_box)
        {}

        std::vector<std::shared_ptr<FGObject>> cubes;
        std::shared_ptr<FGObject> terrain;
        std::shared_ptr<FGObject> player;
        std::shared_ptr<FGObject> cornel_box;
    };

}
