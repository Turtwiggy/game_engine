#pragma once

#include "glm/glm.hpp"

class keyboard_controller
{
public:
    glm::vec3 get_move_dir(const Uint8* keyboard_state)
    {
        glm::vec3 move_dir = glm::vec3(0.f, 0.f, 0.f);

        //Forward and backwards
        if (keyboard_state[SDL_SCANCODE_W])
            move_dir.y = 1.0f;
        else if (keyboard_state[SDL_SCANCODE_S])
            move_dir.y = -1.0f;
        else
            move_dir.y = 0.0f;

        //Left and right
        if (keyboard_state[SDL_SCANCODE_A])
            move_dir.x = -1.0f;
        else if (keyboard_state[SDL_SCANCODE_D])
            move_dir.x = 1.0f;
        else
            move_dir.x = 0.0f;

        //Up and down
        if (keyboard_state[SDL_SCANCODE_SPACE])
            move_dir.z = 1.0f;
        else if (keyboard_state[SDL_SCANCODE_LSHIFT])
            move_dir.z = -1.0f;
        else
            move_dir.z = 0.0f;

        return move_dir;
    }
};

