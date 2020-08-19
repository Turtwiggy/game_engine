#pragma once

#include "glm/glm.hpp"
using namespace glm;

#include "engine/core/game_window.h"

namespace fightingengine {

    struct Camera2D
    {
        vec2 pos;
        float zoom_level = 0;
        int tile_pixels = 0;

        //pix is the expected pixel size for the sprites in the spritesheet
        Camera2D(int pix)
            :tile_pixels(pix)
        {
        }

        vec2 tile_to_screen(GameWindow& win, glm::vec2 tile_pos) const;
        vec2 screen_to_tile(GameWindow& win, glm::vec2 screen_pos) const;
        vec2 world_to_screen(GameWindow& win, glm::vec2 world_pos) const;
        vec2 screen_to_world(GameWindow& win, glm::vec2 screen_pos) const;
        static vec2 tile_to_world(vec2 pos, float tile_pixels);
        //static vec2f world_to_tile(vec2f pos);

        void translate(vec2 amount);

        void zoom(float number_of_levels);

        float calculate_scale() const;
    };
}
