#pragma once

#include "3d/geometry/model.hpp"
#include "3d/transform.hpp"

namespace fightingengine {

    struct GameObject3D
    {
    public:
        GameObject3D(std::shared_ptr<Model> model)
            : model(model)
        {
        }

        std::shared_ptr<Model> model;
        Transform transform;
    };

}
