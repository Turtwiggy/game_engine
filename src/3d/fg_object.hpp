#pragma once

#include "fg_model.hpp"
#include "fg_transform.hpp"

namespace fightinggame {

    struct FGObject
    {
    public:
        FGObject(std::shared_ptr<FGModel> model)
            : model(model)
        {
        }

        std::shared_ptr<FGModel> model;
        FGTransform transform;
    };

}
