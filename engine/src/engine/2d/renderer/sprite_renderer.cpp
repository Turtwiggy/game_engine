
//header
#include "engine/2d/renderer/sprite_renderer.hpp"

//other library headers
#include "GL/glew.h"
#include <glm/gtc/matrix_transform.hpp>

//your project headers
#include "engine/resources/resource_manager.hpp"

namespace fightingengine {

SpriteRenderer::SpriteRenderer(Shader& shader, std::string sprite_sheet_name)
    : spritesheet_name(sprite_sheet_name)
    , plane(2, 2)
{
    spritesheet = ResourceManager::load_texture(("assets/" + spritesheet_name).c_str(), spritesheet_name, true, false);
    
    plane.Finalize();

    shader.set_int("image", spritesheet.id);
}

void SpriteRenderer::draw_sprite
    ( Texture2D& texture
    , Shader& shader
    , const Transform& transform
    , const SpriteHandle& handle )
{
    // prepare transformations
    shader.bind();

    glm::mat4 model = glm::mat4(1.0f);

    // first translate (transformations are: 
    // scale happens first, then rotation, and then final translation happens; reversed order)
    model = glm::translate(model, glm::vec3(transform.position, 0.0f));  

    // move origin of rotation to center of quad
    model = glm::translate(model, glm::vec3(0.5f * transform.scale.x, 0.5f * transform.scale.y, 0.0f)); 
    // then rotate
    model = glm::rotate(model, glm::radians(transform.angle), glm::vec3(0.0f, 0.0f, 1.0f)); 
    // move origin back
    model = glm::translate(model, glm::vec3(-0.5f * transform.scale.x, -0.5f * transform.scale.y, 0.0f));
    // last scale
    model = glm::scale(model, glm::vec3(transform.scale, 1.0f)); 

    this->shader.set_mat4("model", model);
    this->shader.set_vec3("spriteColor", transform.colour.colour);

    glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    render_mesh(plane);
}

void SpriteRenderer::render_mesh(Mesh& mesh)
{
    //bind vao
    glBindVertexArray(mesh.vao);

    if (mesh.Indices.size() > 0)
    {
        glDrawElements(mesh.topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, mesh.Indices.size(), GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(mesh.topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, mesh.Positions.size());
    }

    glBindVertexArray(0);
}

} //namespace fightingengine
