
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
            float angle_radians,
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
  model = glm::translate(model, glm::vec3(world_space, 0.0f));

  model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
  model = glm::rotate(model, angle_radians, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

  model = glm::scale(model, glm::vec3(size, 1.0f));

  shader.set_mat4("model", model);
  shader.set_vec4("sprite_colour", glm::vec4{ color.x, color.y, color.z, 1.0f });

  render_quad();
};

// scale x from [min,max] to [a,b]
float
scale(float x, float min, float max, float a, float b)
{
  return ((b - a) * (x - min)) / (max - min) + a;
}

void
draw_sprite_debug(Camera2D& cam,
                  const glm::ivec2& screen_size,
                  fightingengine::Shader& shader,
                  GameObject2D& game_object,
                  fightingengine::Shader& debug_line_shader,
                  glm::vec4& debug_line_shader_colour,
                  int tex_slot)
{
  draw_sprite(cam,
              screen_size,
              shader,
              game_object.pos,
              game_object.size,
              game_object.angle_radians,
              game_object.colour,
              tex_slot);

  // draw lines
  debug_line_shader.bind();
  debug_line_shader.set_vec4("colour", debug_line_shader_colour);

  glm::vec2 world_pos = game_object.pos - cam.pos;
  glm::vec2 bl_pos = glm::vec2(world_pos.x, world_pos.y + game_object.size.y);
  glm::vec2 tr_pos = glm::vec2(world_pos.x + game_object.size.x, world_pos.y);
  bl_pos.x = scale(bl_pos.x, 0.0f, screen_size.x, -1.0f, 1.0f);
  bl_pos.y = scale(bl_pos.y, 0.0f, screen_size.y, 1.0f, -1.0f);
  tr_pos.x = scale(tr_pos.x, 0.0f, screen_size.x, -1.0f, 1.0f);
  tr_pos.y = scale(tr_pos.y, 0.0f, screen_size.y, 1.0f, -1.0f);

  glColor3f(debug_line_shader_colour.x, debug_line_shader_colour.y, debug_line_shader_colour.z);
  glBegin(GL_LINE_LOOP);
  glVertex2f(bl_pos.x, bl_pos.y);
  glVertex2f(tr_pos.x, bl_pos.y);
  glVertex2f(tr_pos.x, tr_pos.y);
  glVertex2f(bl_pos.x, tr_pos.y);
  glEnd();
}

} // namespace sprite_renderer

} // namespace game2d