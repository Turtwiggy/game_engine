
// header
#include "opengl/sprite_renderer_v1.hpp"

// standard lib headers
#include <array>
#include <iostream>

// other project headers
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

// engine project headers
#include "engine/opengl/util.hpp"
using namespace fightingengine; // used for opengl macro
#include "2d_game_object.hpp"

namespace game2d {

namespace sprite_renderer {

//

// V1 Renderer (Draw Call Per Quad)

//

unsigned int quadVAO = 0;
void
render_quad()
{
  if (quadVAO == 0) {
    float vertices[] = {
      // clang-format off
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
      // clang-format on
    };
    unsigned int VBO;
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void
draw_sprite(const GameObject2D& cam,
            const glm::ivec2& screen_size,
            fightingengine::Shader& shader,
            const GameObject2D& go)
{
  glm::vec2 world_space = gameobject_in_worldspace(cam, go);

  if (gameobject_off_screen(world_space, go.size, screen_size)) {
    return; // skip rendering
  }

  shader.bind();
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(world_space, 0.0f));

  model = glm::translate(model, glm::vec3(0.5f * go.size.x, 0.5f * go.size.y, 0.0f));
  model = glm::rotate(model, go.angle_radians, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::translate(model, glm::vec3(-0.5f * go.size.x, -0.5f * go.size.y, 0.0f));

  model = glm::scale(model, glm::vec3(go.size, 1.0f));

  shader.set_mat4("model", model);
  shader.set_vec4("sprite_colour", glm::vec4{ go.colour.x, go.colour.y, go.colour.z, 1.0f });
  render_quad();
};

} // namespace sprite_renderer

} // namespace game2d