
//header
#include "engine/3d/renderer/renderer_simple.hpp"

//other library files
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

//your project files
#include "engine/resources/resource_manager.hpp"

namespace fightingengine {

    RendererSimple::RendererSimple()
    {
        flat_shader_.load();

        //TODO LOAD MODELS
        //object_ = ResourceManager::load_model("assets/models/lizard_wizard/lizard_wizard.obj", "Object");
    }

    void RendererSimple::update(float delta_time, FlyCamera& camera)
    {
        camera.Update(delta_time);
        glm::mat4 view_projection =  camera.get_view_projection_matrix();

        draw_calls_ = 0;
        flat_shader_.get_shader().bind();
        flat_shader_.get_shader().set_mat4("view_projection", view_projection);
        flat_shader_.get_shader().set_vec3("viewPos", camera.Position);

        // A directional light
        glm::vec3 lightColor(0.9f, 0.9f, 0.9f);
        glm::vec3 diffuseColor = lightColor   * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
        flat_shader_.get_shader().set_vec3("light.ambient", ambientColor);
        flat_shader_.get_shader().set_vec3("light.diffuse", diffuseColor);
        flat_shader_.get_shader().set_vec3("light.specular", 1.0f, 1.0f, 1.0f);
        flat_shader_.get_shader().set_vec3("light.direction", -0.2f, -1.0f, -0.3f);

        // position and scale
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        flat_shader_.get_shader().set_mat4("model", model);

        //note: this is terrible getting all the meshes every frame
        // for (auto i = 0; i < meshes.size(); i++)
        // {   
        //     auto mesh = meshes[i];

        //     //Set material
        //     flat_shader_.get_shader().setVec3("material.ambient", mesh.colour.colour);
        //     flat_shader_.get_shader().setVec3("material.diffuse", mesh.colour.colour);
        //     flat_shader_.get_shader().setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        //     flat_shader_.get_shader().setFloat("material.shininess", 32.0f);

        //     mesh.draw(flat_shader_.get_shader());
        //     draw_calls_ += 1;
        // }

        //Draw a cube
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	
        flat_shader_.get_shader().set_mat4("model", model);
        render_mesh(cube, flat_shader);

        flat_shader_.get_shader().unbind();
    }

    void SimpleRenderer::render_mesh(Mesh* mesh, Shader* shader)
    {
        glBindVertexArray(mesh->vao);
        if (mesh->Indices.size() > 0)
        {
            glDrawElements(mesh->topology == TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, mesh->Indices.size(), GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(mesh->topology == TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, mesh->Positions.size());
        }
    }

} //namespace fightingengine