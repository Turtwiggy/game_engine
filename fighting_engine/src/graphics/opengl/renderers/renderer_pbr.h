#pragma once

#include "graphics/renderer.h"

namespace fightingengine {

    class RendererPBR : public RendererImpl
    {
        void init(int screen_width, int screen_height);
        void draw_pass(draw_scene_desc& desc, const GameState& state);
        void resize(int width, int height);
    };

}
