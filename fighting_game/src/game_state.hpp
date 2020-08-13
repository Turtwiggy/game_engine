#pragma once

#include "engine/3d/game_object_3d.hpp"
using namespace fightingengine;

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
            //std::vector<std::shared_ptr<FGObject>> transforms,
            //std::shared_ptr<FGObject> terrain,
            //std::shared_ptr<FGObject> player,
            std::shared_ptr<GameObject3D> cornel_box )
            //: cubes(transforms)
            //, terrain(terrain)
            //, player(player)
            : cornel_box(cornel_box)
        {}

        //std::vector<std::shared_ptr<FGObject>> cubes;
        //std::shared_ptr<FGObject> terrain;
        //std::shared_ptr<FGObject> player;
        std::shared_ptr<GameObject3D> cornel_box;

        float time;
    };

}
