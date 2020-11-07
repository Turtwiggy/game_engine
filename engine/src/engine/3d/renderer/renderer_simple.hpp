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
    
    void update(float delta_time, FlyCamera& camera, RandomState& rnd);

    void render_mesh(std::shared_ptr<Mesh> mesh);

private:

    void render_scene();

    void draw_skybox(const glm::mat4& view_projection);

private:

    //behaviour to pick objects
    Shader picking_shader_;
    unsigned int picking_fbo_;
    Texture2D picking_colour_tex_;
    Texture2D picking_depth_tex_;

    //rendering shaders / meshes available
    Shader flat_shader_;
    std::shared_ptr<Mesh> cube;
    std::shared_ptr<Mesh> plane;

    Background* background;
    std::shared_ptr<TextureCube> cubemap_;

    uint32_t draw_calls_ = 0;

    // ---------------------
    //some gamestate (that shouldn't live in renderer!)
    std::vector<glm::vec3> cube_pos;

};

} //namespace fightingengine