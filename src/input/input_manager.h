#pragma once

#include "glm/glm.hpp"
#include <SDL2\SDL_keycode.h>

#include <vector>

class InputManager
{
public:
    InputManager()
    {
        down = std::vector<SDL_Keycode>();
        up = std::vector<SDL_Keycode>();
        held = std::vector<SDL_Keycode>();
    }

    void new_frame()
    {
        down.clear();
        up.clear();
    }

    void add_button_down(SDL_Keycode button)
    {
        auto it = std::find(held.begin(), held.end(), button);
        if (it != held.end()) {
            //button is already in held state!
            return;
        }

        down.push_back(button);
        held.push_back(button);
    }

    void add_button_up(SDL_Keycode button)
    {
        up.push_back(button);

        auto it = std::find(held.begin(), held.end(), button);
        if (it != held.end()) {
            //printf("\nbutton found in held array \n");
            held.erase(it);
        }
    }


    bool get_key_down(SDL_Keycode button)
    {
        return std::find(down.begin(), down.end(), button) != down.end();
    }

    bool get_key_up(SDL_Keycode button)
    {
        return std::find(up.begin(), up.end(), button) != up.end();
    }

    bool get_key_held(SDL_Keycode button)
    {
        return std::find(held.begin(), held.end(), button) != held.end();
    }

private:
    //down and up reset every frame
    std::vector<SDL_Keycode> down;
    std::vector<SDL_Keycode> up;
    //contains a sdl_keycode until up is called for it
    std::vector<SDL_Keycode> held;
};
