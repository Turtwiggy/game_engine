#pragma once

#include "engine/geometry/model.hpp"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

namespace fightingengine {

    class Transform3D
    {
    public:
        glm::vec3 Position = glm::vec3{ 0., 0., 0. };
        glm::vec3 Scale = glm::vec3{ 1., 1., 1. };
    };

    struct GameObject3D
    {
    public:
        // GameObject3D(std::shared_ptr<Model> model)
        //     : model(model)
        // {
        // }

        // std::shared_ptr<Model> model;
        Transform3D transform;
    };

}
