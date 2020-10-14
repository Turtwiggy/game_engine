#pragma once

//other project header
#include "glm/glm.hpp"

//your project headers
#include "engine/core/game_window.hpp"

namespace fightingengine {

struct Camera2D
{
    glm::vec2 pos;
    float zoom_level = 0;
    int tile_pixels = 0;

    //pix is the expected pixel size for the sprites in the spritesheet
    Camera2D(int pix)
        :tile_pixels(pix)
    {
    }

    glm::vec2 tile_to_screen(GameWindow& win, glm::vec2 tile_pos) const;
    glm::vec2 screen_to_tile(GameWindow& win, glm::vec2 screen_pos) const;
    glm::vec2 world_to_screen(GameWindow& win, glm::vec2 world_pos) const;
    glm::vec2 screen_to_world(GameWindow& win, glm::vec2 screen_pos) const;
    static glm::vec2 tile_to_world(glm::vec2 pos, float tile_pixels);
    //static glm::vec2f world_to_tile(glm::vec2f pos);

    void translate(glm::vec2 amount);

    void zoom(float number_of_levels);

    float calculate_scale() const;
};

} //namespace fightingengine
