
// header
#include "opengl/sprite_renderer.hpp"

// standard lib headers
#include <array>
#include <iostream>

// other project headers
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

// engine project headers
#include "engine/opengl/util.hpp"
using namespace fightingengine; // used for opengl macro
#include "game/2d_game_object.hpp"

// temp
#include <imgui.h>

namespace game2d {

namespace sprite_renderer {

//

// V2 Renderer (Dynamic Batched Draw Calls)

//

void
init()
{
  //
}

struct Vertex
{
  glm::vec4 pos_and_tex;
  glm::vec4 colour;
  glm::vec4 worldspace_pos;
  // glm::vec4 instance_matrix;
  // float TexID;
};

static Vertex*
create_quad(Vertex* target, float x, float y)
{
  float screen_width = 1366.0f;
  float screen_height = 768.0f;

  glm::vec2 world_size(25.0f, 25.0f);

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(glm::vec2(x, y), 0.0f));
  // model = glm::translate(model, glm::vec3(0.5f * go.size.x, 0.5f * go.size.y, 0.0f));
  // model = glm::rotate(model, go.angle_radians, glm::vec3(0.0f, 0.0f, 1.0f));
  // model = glm::translate(model, glm::vec3(-0.5f * go.size.x, -0.5f * go.size.y, 0.0f));
  model = glm::scale(model, glm::vec3(world_size, 1.0f)); // last scale

  target->pos_and_tex = { 0.0f, 0.0f, 0.0f, 0.0f };
  target->colour = { 0.18f, 0.6f, 0.96f, 1.0f };
  target->worldspace_pos = model * glm::vec4(target->pos_and_tex.x, target->pos_and_tex.y, 0.0f, 1.0f);
  target++;

  target->pos_and_tex = { 1.0f, 0.0f, 1.0f, 0.0f };
  target->colour = { 0.18f, 0.6f, 0.96f, 1.0f };
  target->worldspace_pos = model * glm::vec4(target->pos_and_tex.x, target->pos_and_tex.y, 0.0f, 1.0f);
  target++;

  target->pos_and_tex = { 1.0f, -1.0f, 1.0f, 1.0f };
  target->colour = { 0.98f, 0.6f, 0.96f, 1.0f };
  target->worldspace_pos = model * glm::vec4(target->pos_and_tex.x, target->pos_and_tex.y, 0.0f, 1.0f);
  target++;

  target->pos_and_tex = { 0.0f, -1.0f, 0.0f, 1.0f };
  target->colour = { 0.98f, 0.6f, 0.96f, 1.0f };
  target->worldspace_pos = model * glm::vec4(target->pos_and_tex.x, target->pos_and_tex.y, 0.0f, 1.0f);
  target++;

  return target++;
};

// temp
float pos_x = 1366.0f / 2.0f;
float pos_y = 768.0f / 2.0f;

void
draw_instanced(fightingengine::Shader& shader)
{
  if (VAO == 0) {

    unsigned int EBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &dynamicVBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO); // bind the vao

    glBindBuffer(GL_ARRAY_BUFFER, dynamicVBO);
    glBufferData(GL_ARRAY_BUFFER, max_quad_vert_count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, pos_and_tex));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, colour));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, worldspace_pos));
    // glEnableVertexAttribArray(4);
    // glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offset + 1 * sizeof(glm::vec4)));
    // glEnableVertexAttribArray(5);
    // glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offset + 2 * sizeof(glm::vec4)));
    // glEnableVertexAttribArray(6);
    // glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offset + 3 * sizeof(glm::vec4)));
    // glVertexAttribDivisor(3, 1);
    // glVertexAttribDivisor(4, 1);
    // glVertexAttribDivisor(5, 1);
    // glVertexAttribDivisor(6, 1);

    uint32_t indices[max_quad_index_count];
    uint32_t offset = 0;
    for (int i = 0; i < max_quad_index_count; i += 6) {
      indices[i + 0] = 0 + offset;
      indices[i + 1] = 1 + offset;
      indices[i + 2] = 2 + offset;

      indices[i + 3] = 2 + offset;
      indices[i + 4] = 3 + offset;
      indices[i + 5] = 0 + offset;

      offset += 4;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // unbind vbo and vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  float controls[2] = { pos_x, pos_y };
  ImGui::Begin("Controls");
  ImGui::DragFloat2("Quad pos", controls, 0.1f);
  ImGui::End();
  pos_x = controls[0];
  pos_y = controls[1];

  uint32_t index_count = 0;
  std::array<Vertex, max_quad_vert_count> vertices;
  Vertex* buffer = vertices.data();

  buffer = create_quad(buffer, pos_x, pos_y);
  index_count += 6;
  buffer = create_quad(buffer, 300.0f, 300.0f);
  index_count += 6;
  buffer = create_quad(buffer, 400.0f, 400.0f);
  index_count += 6;

  // Set dynamic vertex buffer & upload data
  glBindBuffer(GL_ARRAY_BUFFER, dynamicVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

  // bind vao & draws
  shader.bind();
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

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

// scale x from [min,max] to [a,b]
float
scale(float x, float min, float max, float a, float b)
{
  return ((b - a) * (x - min)) / (max - min) + a;
}

void
draw_sprite_debug(const GameObject2D& cam,
                  const glm::ivec2& screen_size,
                  fightingengine::Shader& shader,
                  const GameObject2D& game_object,
                  fightingengine::Shader& debug_line_shader,
                  glm::vec4& debug_line_shader_colour)
{
  draw_sprite(cam, screen_size, shader, game_object);

#ifdef WIN32
#ifdef _DEBUG

  // draw lines
  debug_line_shader.bind();
  debug_line_shader.set_vec4("colour", debug_line_shader_colour);

  glm::vec2 world_pos = gameobject_in_worldspace(cam, game_object);
  glm::vec2 bl_pos = glm::vec2(world_pos.x, world_pos.y + game_object.size.y);
  glm::vec2 tr_pos = glm::vec2(world_pos.x + game_object.size.x, world_pos.y);
  bl_pos.x = scale(bl_pos.x, 0.0f, screen_size.x, -1.0f, 1.0f);
  bl_pos.y = scale(bl_pos.y, 0.0f, screen_size.y, 1.0f, -1.0f);
  tr_pos.x = scale(tr_pos.x, 0.0f, screen_size.x, -1.0f, 1.0f);
  tr_pos.y = scale(tr_pos.y, 0.0f, screen_size.y, 1.0f, -1.0f);

  // This API is deprecated, does not work on phones/tablets.
  glBegin(GL_LINE_LOOP);
  glVertex2f(bl_pos.x, bl_pos.y);
  glVertex2f(tr_pos.x, bl_pos.y);
  glVertex2f(tr_pos.x, tr_pos.y);
  glVertex2f(bl_pos.x, tr_pos.y);
  glEnd();

#endif
#endif
}

} // namespace sprite_renderer

} // namespace game2d