#pragma once

#include "3d/fg_model.hpp"

#include "glm/glm.hpp"

namespace fightinggame {

    class FGTransform
    {
    public:

        FGTransform(FGModel& m) : model(m) {}

        glm::vec3 Position = glm::vec3{ 0., 0., 0. };
        glm::vec3 Scale = glm::vec3{ 1., 1., 1. };
        FGModel& model;
    };

}
