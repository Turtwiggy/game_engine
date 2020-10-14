#pragma once

//c++ standard lib headers
#include <vector>

//other library
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_events.h>

namespace fightingengine 
{
    class InputManager
    {
    public:
        InputManager();
        void new_frame();

        //keyboard

        void add_button_down(SDL_Keycode button);
        bool get_key_down(SDL_Keycode button);
        bool get_key_held(SDL_Scancode button);

        //mouse

        void add_mouse_down(SDL_MouseButtonEvent& mouse_e);
        bool get_mouse_lmb_held();
        bool get_mouse_rmb_held();
        bool get_mouse_mmb_held();

    private:

        //Keyboard state
        //down and up reset every frame
        std::vector<SDL_Keycode> down;
        const Uint8* state;

    };

}
