#pragma once

//c++ standard lib headers
#include <memory>
#include <vector> //remove this eventually!

//other library files
#include <glm/glm.hpp>

//your project headers
#include "engine/core/maths/random.hpp"
#include "engine/graphics/shaders/flat_shader.hpp"
#include "engine/3d/camera/camera.hpp"
#include "engine/3d/camera/fly_camera.hpp"
#include "engine/scene/scene_node.hpp"
#include "engine/scene/background.hpp"
#include "engine/mesh/primitives.hpp"

namespace fightingengine
{

class RendererSimple
{
public:
    RendererSimple(RandomState& rnd);
    
    void update(float delta_time, FlyCamera& camera, RandomState& rnd);

    void render_mesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader);

private:

    void draw_skybox(const glm::mat4& view_projection);

private:
    //std::shared_ptr<Model> object_;

    std::shared_ptr<Shader> flat_shader_;
    std::shared_ptr<Mesh> cube;
    std::shared_ptr<Mesh> plane;

    Background* background;
    std::shared_ptr<TextureCube> cubemap_;

    //some gamestate
    std::vector<glm::vec3> cube_pos;


    uint32_t draw_calls_ = 0;
};

} //namespace fightingengine