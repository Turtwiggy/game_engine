#include "pch.hpp"

// header
#include "batch_triangle.hpp"

#include "engine/deps/opengl.hpp"
#include "engine/opengl/util.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace engine {

namespace tri_renderer {

void
TriangleRenderer::draw_sprite(const TriangleDescriptor& r, const Shader& s)
{
  if (data.index_count >= max_tri_index_count) {
    end_batch();
    flush(s);
    begin_batch();
  }

  // const float tex_slot = static_cast<float>(r.tex_unit);
  glm::mat4 model = glm::mat4(1.0f);
  const glm::vec4 a_colour = { r.point_0_colour.r, r.point_0_colour.g, r.point_0_colour.b, r.point_0_colour.a };
  const glm::vec4 b_colour = { r.point_1_colour.r, r.point_1_colour.g, r.point_1_colour.b, r.point_1_colour.a };
  const glm::vec4 c_colour = { r.point_2_colour.r, r.point_2_colour.g, r.point_2_colour.b, r.point_2_colour.a };

  data.buffer_ptr->pos_and_uv = { r.point_0.x, r.point_0.y, r.uv_0.x, r.uv_0.y };
  data.buffer_ptr->colour = a_colour;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.buffer_ptr->pos_and_uv = { r.point_1.x, r.point_1.y, r.uv_1.x, r.uv_1.y };
  data.buffer_ptr->colour = b_colour;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.buffer_ptr->pos_and_uv = { r.point_2.x, r.point_2.y, r.uv_2.x, r.uv_2.y };
  data.buffer_ptr->colour = c_colour;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.index_count += 3;
  data.tri_vertex += 3;
}

void
TriangleRenderer::init()
{
#if defined(_DEBUG)
  CHECK_OPENGL_ERROR(5); // check any error before this stage
#endif

  data.buffer = new TriVertex[max_tri_vert_count];

  glGenVertexArrays(1, &data.VAO);
  glGenBuffers(1, &data.VBO);
  glGenBuffers(1, &data.EBO);

  glBindVertexArray(data.VAO); // bind the vao

  glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
  glBufferData(GL_ARRAY_BUFFER, max_tri_vert_count * sizeof(TriVertex), nullptr, GL_DYNAMIC_DRAW); // dynamic

  // specific to the game...

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(TriVertex), (const void*)offsetof(TriVertex, pos_and_uv));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TriVertex), (const void*)offsetof(TriVertex, colour));

  // model
  {
    int va0 = 2;

    glEnableVertexAttribArray(va0);
    glVertexAttribPointer(va0, 4, GL_FLOAT, GL_FALSE, sizeof(TriVertex), (const void*)offsetof(TriVertex, model));

    glEnableVertexAttribArray(va0 + 1);
    glVertexAttribPointer(
      va0 + 1, 4, GL_FLOAT, GL_FALSE, sizeof(TriVertex), (const void*)(offsetof(TriVertex, model) + 1 * sizeof(glm::vec4)));

    glEnableVertexAttribArray(va0 + 2);
    glVertexAttribPointer(
      va0 + 2, 4, GL_FLOAT, GL_FALSE, sizeof(TriVertex), (const void*)(offsetof(TriVertex, model) + 2 * sizeof(glm::vec4)));

    glEnableVertexAttribArray(va0 + 3);
    glVertexAttribPointer(
      va0 + 3, 4, GL_FLOAT, GL_FALSE, sizeof(TriVertex), (const void*)(offsetof(TriVertex, model) + 3 * sizeof(glm::vec4)));
  }

  uint32_t indices[max_tri_index_count];
  for (uint32_t i = 0; i < max_tri_index_count; i += 3) {
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
  // glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data.buffer);
}

void
TriangleRenderer::flush(const Shader& shader)
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
TriangleRenderer::reset_vert_count()
{
  data.tri_vertex = 0;
};

void
TriangleRenderer::end_frame()
{
  data.draw_calls = 0;
};

int
TriangleRenderer::draw_calls() const
{
  return data.draw_calls;
};

} // namespace tri_renderer

} // namespace engine