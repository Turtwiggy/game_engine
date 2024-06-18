// your header
#include "batch_triangle_fan.hpp"

// my libs
#include "batch_renderer.hpp"

// other lib headers
#include <GL/glew.h>

namespace engine {

namespace triangle_fan_renderer {

static RenderData<Vertex> data;

void
TriangleFanRenderer::add_point_to_fan(const glm::ivec2 point, Shader& s)
{
  // if (data.index_count >= max_quad_index_count) {
  //   end_batch();
  //   flush(s);
  //   begin_batch();
  // }

  glm::mat4 model = glm::mat4(1.0f);
  // model = glm::translate(model, glm::vec3(glm::vec2(worldspace_pos.x, worldspace_pos.y), 0.0f));
  // model = glm::translate(model, glm::vec3(0.5f * go.render_size.x, 0.5f * go.render_size.y, 0.0f));
  // model = glm::rotate(model, go.angle_radians, glm::vec3(0.0f, 0.0f, 1.0f));
  // model = glm::translate(model, glm::vec3(-0.5f * go.render_size.x, -0.5f * go.render_size.y, 0.0f));
  // model = glm::scale(model, glm::vec3(go.render_size, 1.0f));

  data.buffer_ptr->pos = { point.x, point.y, 0.0f, 0.0f };
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.index_count += 1;
  data.quad_vertex += 1;
}

void
TriangleFanRenderer::init()
{
  data.buffer = new Vertex[max_quad_vert_count];

  glGenVertexArrays(1, &data.VAO);
  glGenBuffers(1, &data.VBO);
  glGenBuffers(1, &data.EBO);
  glBindVertexArray(data.VAO); // bind the vao

  glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
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
  for (int i = 0; i < max_quad_index_count; i++) {
    indices[i] = i;
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // unbind vbo and vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
};

void
TriangleFanRenderer::shutdown()
{
  glDeleteVertexArrays(1, &data.VAO);
  glDeleteBuffers(1, &data.VBO);
  glDeleteBuffers(1, &data.EBO);

  delete[] data.buffer;
}

// upload data to vbo
void
TriangleFanRenderer::end_batch()
{
  GLsizeiptr size = (uint8_t*)data.buffer_ptr - (uint8_t*)data.buffer;
  // Set dynamic vertex buffer & upload data
  glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data.buffer);
}

// submit quads for a drawcall
void
TriangleFanRenderer::flush(Shader& s)
{
  s.bind();

  glBindVertexArray(data.VAO);
  glDrawElements(GL_TRIANGLE_FAN, data.index_count, GL_UNSIGNED_INT, nullptr);

  data.draw_calls += 1;
  data.index_count = 0;

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void
TriangleFanRenderer::begin_batch()
{
  data.buffer_ptr = data.buffer;
}

void
TriangleFanRenderer::reset_quad_vert_count()
{
  data.quad_vertex = 0;
}

void
TriangleFanRenderer::end_frame()
{
  data.draw_calls = 0;
}

int
TriangleFanRenderer::draw_calls()
{
  return data.draw_calls;
}

} // namespace triangle_fan_renderer

} // namespace engine