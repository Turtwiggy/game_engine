// your header
#include "opengl/triangle_fan.hpp"

// c++ lib headers
#include <vector>

// other lib headers
#include <GL/glew.h>

namespace game2d {

namespace triangle_fan_renderer {

struct Vertex
{
  glm::vec4 pos;
  glm::mat4 model;
};

static const size_t max_quad = 5000;
static const size_t max_quad_vert_count = max_quad * 4;
static const size_t max_quad_index_count = max_quad * 6;

struct renderer_data
{
  unsigned int VAO = 0;
  unsigned int VBO = 0;
  unsigned int EBO = 0;

  uint32_t index_count = 0;
  int quad_vertex = 0;

  Vertex* buffer;
  Vertex* buffer_ptr;

  // stats
  int draw_calls = 0;
};
static renderer_data s_data;

void
reset_quad_vert_count()
{
  s_data.quad_vertex = 0;
}
int
get_draw_calls()
{
  return s_data.draw_calls;
}
int
get_quad_count()
{
  return s_data.quad_vertex;
}
void
end_frame()
{
  s_data.draw_calls = 0;
}

void
init()
{
  s_data.buffer = new Vertex[max_quad_vert_count];

  glGenVertexArrays(1, &s_data.VAO);
  glGenBuffers(1, &s_data.VBO);
  glGenBuffers(1, &s_data.EBO);
  glBindVertexArray(s_data.VAO); // bind the vao

  glBindBuffer(GL_ARRAY_BUFFER, s_data.VBO);
  glBufferData(GL_ARRAY_BUFFER, max_quad_vert_count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW); // dynamic

  // attribs

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, pos));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, model));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
    2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, model) + 1 * sizeof(glm::vec4)));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(
    3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, model) + 2 * sizeof(glm::vec4)));

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(
    4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, model) + 3 * sizeof(glm::vec4)));

  // indicies

  uint32_t indices[max_quad_index_count];
  uint32_t index_offset = 0;
  for (int i = 0; i < max_quad_index_count; i++) {
    indices[i] = index_offset;

    index_offset += 1;
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_data.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // unbind vbo and vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
};
void
shutdown()
{
  glDeleteVertexArrays(1, &s_data.VAO);
  glDeleteBuffers(1, &s_data.VBO);
  glDeleteBuffers(1, &s_data.EBO);

  delete[] s_data.buffer;
}

// upload data to vbo
void
end_batch()
{
  GLsizeiptr size = (uint8_t*)s_data.buffer_ptr - (uint8_t*)s_data.buffer;
  // Set dynamic vertex buffer & upload data
  glBindBuffer(GL_ARRAY_BUFFER, s_data.VBO);
  // glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, s_data.buffer);
}
// submit quads for a drawcall
void
flush(fightingengine::Shader& shader)
{
  shader.bind();

  glBindVertexArray(s_data.VAO);
  glDrawElements(GL_TRIANGLE_FAN, s_data.index_count, GL_UNSIGNED_INT, nullptr);

  s_data.draw_calls += 1;
  s_data.index_count = 0;

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}
void
begin_batch()
{
  s_data.buffer_ptr = s_data.buffer;
}

void
add_point_to_fan(const GameObject2D& cam, fightingengine::Shader& shader, const glm::vec2 point)
{
  if (s_data.index_count >= max_quad_index_count) {
    end_batch();
    flush(shader);
    begin_batch();
  }

  glm::vec2 worldspace_pos = point - cam.pos;

  glm::mat4 model = glm::mat4(1.0f);
  // model = glm::translate(model, glm::vec3(glm::vec2(worldspace_pos.x, worldspace_pos.y), 0.0f));
  // model = glm::translate(model, glm::vec3(0.5f * go.render_size.x, 0.5f * go.render_size.y, 0.0f));
  // model = glm::rotate(model, go.angle_radians, glm::vec3(0.0f, 0.0f, 1.0f));
  // model = glm::translate(model, glm::vec3(-0.5f * go.render_size.x, -0.5f * go.render_size.y, 0.0f));
  // model = glm::scale(model, glm::vec3(go.render_size, 1.0f));

  s_data.buffer_ptr->pos = { point.x, point.y, 0.0f, 0.0f };
  s_data.buffer_ptr->model = model;
  s_data.buffer_ptr++;

  s_data.index_count += 1;
  s_data.quad_vertex += 1;
}

} // namespace triangle_fan_renderer

} // namespace game2d