#pragma once

//c++ standard lib headers
#include <memory>
#include <vector> //remove this when remove cube_pos

//other library files
#include <glm/glm.hpp>

//your project headers
#include "engine/core/maths/random.hpp"
#include "engine/graphics/framebuffer.hpp"
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
    RendererSimple(RandomState& rnd, int width, int height);
    
    void update(
        float delta_time, 
        FlyCamera& camera, 
        RandomState& rnd, 
        const std::vector<glm::vec3>& cube_pos
    );

    void render_mesh(std::shared_ptr<Mesh> mesh);

    int get_draw_calls();

private:

    void draw_skybox(const glm::mat4& view_projection);

private:

    //rendering shaders / meshes available
    Shader flat_shader_;
    std::shared_ptr<Mesh> cube;
    std::shared_ptr<Mesh> plane;

    Background* background;
    std::shared_ptr<TextureCube> cubemap_;

    int draw_calls_ = 0;

    //initial width and height
    int width_, height_ = 0;

    // ---------------------
    //some gamestate (that shouldn't live in renderer!)

};

} //namespace fightingengine