#pragma once

//your project headers
#include "engine/graphics/shaders/flat_shader.hpp"
#include "engine/3d/model.hpp"
#include "engine/3d/camera/camera.hpp"
#include "engine/3d/camera/fly_camera.hpp"
#include "engine/graphics/primitives/primitives.hpp"

#include <memory>

namespace fightingengine
{
    class RendererSimple
    {
    public:
        RendererSimple();
        
        void update(float delta_time, FlyCamera& camera);

    private:
        FlatShader flat_shader_;
        std::shared_ptr<Model> object_;
        primitives::Cube cube;

        uint32_t draw_calls_ = 0;
    };
}