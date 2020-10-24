
//header
#include "engine/3d/renderer/renderer_simple.hpp"

//your project files
#include "engine/resources/resource_manager.hpp"

namespace fightingengine {

    RendererSimple::RendererSimple()
    {
        flat_shader_.load();

        //TODO LOAD MODELS
        cube_ = ResourceManager::load_model("assets/models/cube/cube.obj", "Cube");
    }

    void RendererSimple::update(const Camera& cam, int width, int height)
    {
        glm::mat4 view_projection = cam.get_view_projection_matrix(width, height);

        flat_shader_.get_shader().bind();
        flat_shader_.get_shader().setMat4("view_projection", view_projection);

        //Draw all objects here



        flat_shader_.get_shader().unbind();
    }

} //namespace fightingengine