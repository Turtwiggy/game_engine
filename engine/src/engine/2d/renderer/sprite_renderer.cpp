
//header
#include "engine/2d/renderer/sprite_renderer.hpp"

//other library headers
#include "GL/glew.h"
#include <glm/gtc/matrix_transform.hpp>

//your project headers
#include "engine/resources/resource_manager.hpp"

namespace fightingengine {

// void SpriteRenderer::draw_sprite
//     ( Texture2D& texture
//     , Shader& shader
//     , const Transform& transform
//     , const SpriteHandle& handle )
// {
//     // prepare transformations
//     shader.bind();

//     glm::mat4 model = glm::mat4(1.0f);
//     model = glm::translate(model, glm::vec3(transform.position, 0.0f));  

//     // move origin of rotation to center of quad
//     model = glm::translate(model, glm::vec3(0.5f * transform.scale.x, 0.5f * transform.scale.y, 0.0f)); 
//     // then rotate
//     model = glm::rotate(model, glm::radians(transform.angle), glm::vec3(0.0f, 0.0f, 1.0f)); 
//     // move origin back
//     model = glm::translate(model, glm::vec3(-0.5f * transform.scale.x, -0.5f * transform.scale.y, 0.0f));
//     // scale
//     model = glm::scale(model, glm::vec3(transform.scale, 1.0f)); 

//     shader.set_mat4("model", model);
//     shader.set_vec3("spriteColor", transform.colour.colour);

//     glActiveTexture(GL_TEXTURE0);
//     texture.Bind();

//     render_mesh(plane);

//     texture.Unbind();
// }

} //namespace fightingengine
