
// header
#include "sprite_renderer.hpp"

// standard lib headers
#include <iostream>

// other project headers
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

// engine project headers
#include "engine/opengl/texture.hpp"

namespace game2d {

namespace sprite_renderer {

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
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

// void
// draw_sprite(Shader& shader, GameObject& game_object)
// {
//   draw_sprite(shader, game_object.transform, game_object.tex_slot);
// }

// void
// draw_sprite(Shader& shader, Transform& t, int tex_slot)
// {
//   draw_sprite(shader, t.position, t.scale, t.angle, t.colour, tex_slot);
// }

void
draw_sprite(Camera2D& cam,
            const glm::ivec2& screen_size,
            fightingengine::Shader& shader,
            glm::vec2 position,
            glm::vec2 size,
            float angle,
            glm::vec3 color,
            int tex_slot)
{
  glm::vec2 tl_visible = glm::vec2(0.0f, 0.0f);
  glm::vec2 br_visible = screen_size;
  // float camera_scale = cam.calculate_scale();

  glm::vec2 world_space = position - cam.pos;

  // clang-format off
  if (
      // left of screen
      world_space.x + size.x < tl_visible.x ||
      // top of screen
      world_space.y + size.y < tl_visible.y || 
      // right of screen
      world_space.x > br_visible.x ||
      // bottom of screen
      world_space.y > br_visible.y) {
    // std::cout << "skipping rendering of object";
    return;
  }
  // clang-format on

  // render object
  shader.bind();

  glm::mat4 model = glm::mat4(1.0f);

  // move origin of rotation to center of quad
  model = glm::translate(model, glm::vec3(world_space, 0.0f));

  model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
  // then rotate
  model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
  // move origin back
  model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

  // then scale
  model = glm::scale(model, glm::vec3(size, 1.0f));

  shader.set_mat4("model", model);
  shader.set_vec4("sprite_colour", glm::vec4{ color.x, color.y, color.z, 1.0f });

  render_quad();
};

} // namespace sprite_renderer

} // namespace game2d