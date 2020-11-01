#pragma once

//your project headers
#include "engine/graphics/shaders/flat_shader.hpp"
#include "engine/3d/camera/camera.hpp"
#include "engine/3d/camera/fly_camera.hpp"
#include "engine/mesh/model.hpp"
#include "engine/mesh/primitives.hpp"

#include <memory>

namespace fightingengine
{

class RendererSimple
{
public:
    RendererSimple();
    
    void update(float delta_time, FlyCamera& camera);

    void render_mesh(Mesh* mesh, Shader shader);

private:
    //std::shared_ptr<Model> object_;

    Shader flat_shader_;
    Mesh* cube;
    Mesh* plane;

    uint32_t draw_calls_ = 0;
};

} //namespace fightingengine