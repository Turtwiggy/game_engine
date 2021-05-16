
// header
#include "sprite_renderer.hpp"

// standard lib headers
#include <iostream>

// other project headers
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

// engine project headers
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
using namespace fightingengine; // used for opengl macro

// temp
#include <imgui.h>

namespace game2d {

glm::vec2
gameobject_in_worldspace(const GameObject2D& camera, const GameObject2D& go)
{
  return go.pos - camera.pos;
}

bool
gameobject_off_screen(glm::vec2 pos, glm::vec2 size, const glm::ivec2& screen_size)
{
  glm::vec2 tl_visible = glm::vec2(0.0f, 0.0f);
  glm::vec2 br_visible = screen_size;
  if (
    // left of screen
    pos.x + size.x < tl_visible.x ||
    // top of screen
    pos.y + size.y < tl_visible.y ||
    // right of screen
    pos.x > br_visible.x ||
    // bottom of screen
    pos.y > br_visible.y) {
    return true;
  }
  return false;
}

namespace sprite_renderer {

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

static std::array<Vertex, 4>
create_quad(float x, float y)
{
  float screen_width = 1366.0f;
  float screen_height = 768.0f;
  glm::mat4 projection = glm::ortho(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);

  glm::vec2 world_size(25.0f, 25.0f);

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(glm::vec2(x, y), 0.0f));
  // model = glm::translate(model, glm::vec3(0.5f * go.size.x, 0.5f * go.size.y, 0.0f));
  // model = glm::rotate(model, go.angle_radians, glm::vec3(0.0f, 0.0f, 1.0f));
  // model = glm::translate(model, glm::vec3(-0.5f * go.size.x, -0.5f * go.size.y, 0.0f));
  model = glm::scale(model, glm::vec3(world_size, 1.0f)); // last scale

  Vertex v0;
  v0.pos_and_tex = { 0.0f, 0.0f, 0.0f, 0.0f };
  v0.colour = { 0.18f, 0.6f, 0.96f, 1.0f };
  v0.worldspace_pos = projection * model * glm::vec4(v0.pos_and_tex.x, v0.pos_and_tex.y, 0.0f, 1.0f);

  Vertex v1;
  v1.pos_and_tex = { 1.0f, 0.0f, 1.0f, 0.0f };
  v1.colour = { 0.18f, 0.6f, 0.96f, 1.0f };
  v1.worldspace_pos = projection * model * glm::vec4(v1.pos_and_tex.x, v1.pos_and_tex.y, 0.0f, 1.0f);

  Vertex v2;
  v2.pos_and_tex = { 1.0f, -1.0f, 1.0f, 1.0f };
  v2.colour = { 0.18f, 0.6f, 0.96f, 1.0f };
  v2.worldspace_pos = projection * model * glm::vec4(v2.pos_and_tex.x, v2.pos_and_tex.y, 0.0f, 1.0f);

  Vertex v3;
  v3.pos_and_tex = { 0.0f, -1.0f, 0.0f, 1.0f };
  v3.colour = { 0.18f, 0.6f, 0.96f, 1.0f };
  v3.worldspace_pos = projection * model * glm::vec4(v3.pos_and_tex.x, v3.pos_and_tex.y, 0.0f, 1.0f);

  return { v0, v1, v2, v3 };
};

int draw_call_quad_amount = 1000;
unsigned int VAO, dynamicVBO = 0;
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * draw_call_quad_amount, nullptr, GL_DYNAMIC_DRAW);

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

    uint32_t indices[] = {
      // clang-format off
      0, 1, 2, 2, 3, 0,
      4, 5, 6, 6, 7, 4
      // clang-format on
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // unbind vbo and vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  shader.bind();

  float controls[2] = { pos_x, pos_y };
  ImGui::Begin("Controls");
  ImGui::DragFloat2("Quad pos", controls, 0.1f);
  ImGui::End();
  pos_x = controls[0];
  pos_y = controls[1];

  auto q0 = create_quad(pos_x, pos_y);
  auto q1 = create_quad(1366.0f / 2.0f, 768.0f / 2.0f);
  Vertex vertices[8];
  memcpy(vertices, q0.data(), q0.size() * sizeof(Vertex));
  memcpy(vertices + q0.size(), q1.data(), q1.size() * sizeof(Vertex));

  // Set dynamic vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, dynamicVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glBindVertexArray(VAO);
  glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0, draw_call_quad_amount);

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

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