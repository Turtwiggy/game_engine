#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/vec3.hpp> 

struct GameState
{
    glm::vec3 cube_pos = glm::vec3(0.f, 0.f, 0.f);
};
