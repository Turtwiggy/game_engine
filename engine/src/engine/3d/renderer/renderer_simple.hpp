#pragma once

//your project headers
#include "engine/graphics/shaders/flat_shader.hpp"
#include "engine/3d/model.hpp"
#include "engine/3d/camera.hpp"
#include "engine/graphics/primitives/primitives.hpp"

#include <memory>

namespace fightingengine
{
    class RendererSimple
    {
    public:
        RendererSimple();
        
        void update(const Camera& cam, int width, int height);

    private:
        FlatShader flat_shader_;
        std::shared_ptr<Model> object_;
        
        primitives::Cube cube;


        uint32_t draw_calls_ = 0;
    };
}