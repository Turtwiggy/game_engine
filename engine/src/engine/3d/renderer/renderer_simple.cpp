
//header
#include "engine/3d/renderer/renderer_simple.hpp"

//your project files
#include "engine/resources/resource_manager.hpp"

namespace fightingengine {

    RendererSimple::RendererSimple()
    {
        flat_shader_.load();
        cube.init();

        //TODO LOAD MODELS
        object_ = ResourceManager::load_model("assets/models/lizard_wizard/lizard_wizard.obj", "Object");
    }

    void RendererSimple::update(const Camera& cam, int width, int height)
    {
        glm::mat4 view_projection = cam.get_view_projection_matrix(width, height);

        draw_calls_ = 0;
        flat_shader_.get_shader().bind();
        flat_shader_.get_shader().setMat4("view_projection", view_projection);
        flat_shader_.get_shader().setVec3("viewPos", cam.Position);

        // A directional light
        glm::vec3 lightColor(0.9f, 0.9f, 0.9f);
        glm::vec3 diffuseColor = lightColor   * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
        flat_shader_.get_shader().setVec3("light.ambient", ambientColor);
        flat_shader_.get_shader().setVec3("light.diffuse", diffuseColor);
        flat_shader_.get_shader().setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        flat_shader_.get_shader().setVec3("light.direction", -0.2f, -1.0f, -0.3f);

        // position and scale
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        flat_shader_.get_shader().setMat4("model", model);
        object_->draw(flat_shader_.get_shader(), draw_calls_);

        //Draw a cube
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	
        flat_shader_.get_shader().setMat4("model", model);
        cube.draw();

        flat_shader_.get_shader().unbind();
    }

} //namespace fightingengine