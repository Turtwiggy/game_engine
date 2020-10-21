
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
    , shader(shader)
{
    spritesheet = ResourceManager::load_texture(("assets/" + spritesheet_name).c_str(), spritesheet_name, true, false);

    // configure VAO/VBO
    unsigned int VBO;
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &this->quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(this->quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shader.setInt("image", spritesheet.id);
}

SpriteRenderer::~SpriteRenderer()
{
    glDeleteVertexArrays(1, &this->quadVAO);
}

void SpriteRenderer::draw_sprite
    ( Texture2D& texture
    , const RenderDescriptor& desc
    , const SpriteHandle& handle )
{
    // prepare transformations
    this->shader.bind();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(desc.position, 0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

    model = glm::translate(model, glm::vec3(0.5f * desc.scale.x, 0.5f * desc.scale.y, 0.0f)); // move origin of rotation to center of quad
    model = glm::rotate(model, glm::radians(desc.angle), glm::vec3(0.0f, 0.0f, 1.0f)); // then rotate
    model = glm::translate(model, glm::vec3(-0.5f * desc.scale.x, -0.5f * desc.scale.y, 0.0f)); // move origin back

    model = glm::scale(model, glm::vec3(desc.scale, 1.0f)); // last scale

    this->shader.setMat4("model", model);

    // render textured quad
    this->shader.setVec3("spriteColor", desc.colour.colour);

    glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    glBindVertexArray(this->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

} //namespace fightingengine
