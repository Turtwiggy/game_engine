#pragma once

//your project headers
#include "engine/graphics/shaders/flat_shader.hpp"
#include "engine/3d/camera/camera.hpp"
#include "engine/3d/camera/fly_camera.hpp"
#include "engine/scene/scene.hpp"
#include "engine/scene/scene_node.hpp"
#include "engine/scene/background.hpp"
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

    void render_mesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader);

private:
    //std::shared_ptr<Model> object_;

    std::shared_ptr<Shader> flat_shader_;
    std::shared_ptr<Mesh> cube;
    std::shared_ptr<Mesh> plane;

    Background* background;
    std::shared_ptr<TextureCube> cubemap_;

    uint32_t draw_calls_ = 0;
};

} //namespace fightingengine