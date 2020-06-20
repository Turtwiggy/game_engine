#pragma once

#include "3d/assimp_obj_loader.h"

#include "glm/glm.hpp"
#include <functional>

using namespace fightinggame;

class FGTransform
{
public:

    FGTransform(FGModel& m) : model(m) {}

    glm::vec3 Position;
    glm::vec3 Scale;
    FGModel& model;
};
