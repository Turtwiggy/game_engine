
// header
#include "batch_triangle.hpp"

// my libs
#include "batch_renderer.hpp"

// other project headers
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace engine {

namespace triangle_renderer {

static RenderData<Vertex> data;

void
TriangleRenderer::draw_triangle(const TriangleDescriptor& r, Shader& s)
{
  if (data.index_count >= max_quad_index_count) {
    end_batch();
    flush(s);
    begin_batch();
  }

  const int& tex_slot = r.tex_slot;
  const glm::ivec2 sprite_offset = { 0, 0 };
  glm::mat4 model = glm::mat4(1.0f);

  data.buffer_ptr->pos_and_uv = { r.point_0.x, r.point_0.y, 0.0f, 0.0f };
  data.buffer_ptr->colour = r.point_0_colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->tex_slot = tex_slot;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.buffer_ptr->pos_and_uv = { r.point_1.x, r.point_1.y, 0.0f, 0.0f };
  data.buffer_ptr->colour = r.point_1_colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->tex_slot = tex_slot;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.buffer_ptr->pos_and_uv = { r.point_2.x, r.point_2.y, 0.0f, 0.0f };
  data.buffer_ptr->colour = r.point_2_colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->tex_slot = tex_slot;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.index_count += 3;
  data.quad_vertex += 3;
}

void
TriangleRenderer::init()
{
  data.buffer = new Vertex[max_quad_vert_count];

  glGenVertexArrays(1, &data.VAO);
  glGenBuffers(1, &data.VBO);
  glGenBuffers(1, &data.EBO);
  glBindVertexArray(data.VAO); // bind the vao

  glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
  glBufferData(GL_ARRAY_BUFFER, max_quad_vert_count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW); // dynamic

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, pos_and_uv));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, colour));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, sprite_offset));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_slot));

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, model));

  glEnableVertexAttribArray(5);
  glVertexAttribPointer(
    5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, model) + 1 * sizeof(glm::vec4)));

  glEnableVertexAttribArray(6);
  glVertexAttribPointer(
    6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, model) + 2 * sizeof(glm::vec4)));

  glEnableVertexAttribArray(7);
  glVertexAttribPointer(
    7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, model) + 3 * sizeof(glm::vec4)));

  uint32_t indices[max_quad_index_count];
  for (int i = 0; i < max_quad_index_count; i += 3) {
    indices[i + 0] = 0 + i;
    indices[i + 1] = 1 + i;
    indices[i + 2] = 2 + i;
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // unbind vbo and vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
};

void
TriangleRenderer::shutdown()
{
  glDeleteVertexArrays(1, &data.VAO);
  glDeleteBuffers(1, &data.VBO);
  glDeleteBuffers(1, &data.EBO);

  delete[] data.buffer;
}

// upload data to vbo
void
TriangleRenderer::end_batch()
{
  GLsizeiptr size = (uint8_t*)data.buffer_ptr - (uint8_t*)data.buffer;
  // Set dynamic vertex buffer & upload data
  glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data.buffer);
}

void
TriangleRenderer::flush(Shader& shader)
{
  shader.bind();

  if (data.index_count > 0) {
    glBindVertexArray(data.VAO);
    glDrawElements(GL_TRIANGLES, data.index_count, GL_UNSIGNED_INT, nullptr);
    data.draw_calls += 1;
    data.index_count = 0;
  }

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void
TriangleRenderer::begin_batch()
{
  data.buffer_ptr = data.buffer;
}

void
TriangleRenderer::reset_quad_vert_count()
{
  data.quad_vertex = 0;
}

void
TriangleRenderer::end_frame()
{
  data.draw_calls = 0;
}

void
TriangleRenderer::draw(entt::registry& registry, Shader& shader)
{
  TriangleRenderer::reset_quad_vert_count();
  TriangleRenderer::begin_batch();
  {
    for (auto& interface : interfaces) {
      const auto& tris = interface->get_triangles(registry);
      for (const auto& triangle : tris) {
        draw_triangle(triangle, shader);
      }
    }
  }
  TriangleRenderer::end_batch();
  TriangleRenderer::flush(shader);
};

int
TriangleRenderer::draw_calls()
{
  return data.draw_calls;
}

} // namespace triangle_renderer

} // namespace engine