#pragma once

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_events.h>

#include <vector>

namespace fightingengine {

    class InputManager
    {
    public:
        InputManager()
        {
            down = std::vector<SDL_Keycode>();
            state = SDL_GetKeyboardState(NULL);
        }

        void new_frame()
        {
            down.clear();
        }

        //keyboard

        void add_button_down(SDL_Keycode button)
        {
            down.push_back(button);
        }

        bool get_key_down(SDL_Keycode button)
        {
            return std::find(down.begin(), down.end(), button) != down.end();
        }

        bool get_key_held(SDL_Scancode button)
        {
            return state[button];
        }

        //mouse

        void add_mouse_down(SDL_MouseButtonEvent& mouse_e)
        {
            if (mouse_e.button == SDL_BUTTON_LEFT)
            {
                printf("left mouse clicked");
            }
            if (mouse_e.button == SDL_BUTTON_RIGHT)
            {
                printf("right mouse clicked");
            }
            if (mouse_e.button == SDL_BUTTON_MIDDLE)
            {
                printf("middle mouse clicked");
            }
        }

        bool get_mouse_lmb_held()
        {
            return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
        }

        bool get_mouse_rmb_held()
        {
            return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
        }

        bool get_mouse_mmb_held()
        {
            return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE);
        }

    private:

        //Keyboard state

        //down and up reset every frame
        std::vector<SDL_Keycode> down;

        const Uint8* state; //use this for keyboard state

    };

}
