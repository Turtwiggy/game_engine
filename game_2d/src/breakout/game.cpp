
//header
#include "breakout/game.hpp"

namespace game2d 
{
    
void draw_sprite(
    fightingengine::Shader& shader,
    unsigned int VAO,
    fightingengine::Texture2D& texture, 
    glm::vec2 position, 
    glm::vec2 size, 
    float rotate, 
    glm::vec3 color )
{
    shader.bind();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); // move origin of rotation to center of quad
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f)); // then rotate
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f)); // move origin back

    model = glm::scale(model, glm::vec3(size, 1.0f)); // last scale

    shader.set_mat4( "model", model );
    shader.set_vec3( "spriteColor", color );

    texture.Bind();

    //render_mesh(data.plane);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    //data.sprite_texture.Unbind();
};


} //namespace game2d