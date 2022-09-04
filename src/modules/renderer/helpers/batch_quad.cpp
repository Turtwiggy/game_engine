
// header
#include "batch_quad.hpp"

// my libs
#include "batch_renderer.hpp"

// other project headers
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {

namespace quad_renderer {

static RenderData<Vertex> data;

void
QuadRenderer::draw_sprite(const RenderDescriptor& r, const glm::mat4& model, const Shader& s)
{
  if (data.index_count >= max_quad_index_count) {
    end_batch();
    flush(s);
    begin_batch();
  }

  const glm::vec2 sprite_offset = { r.sprite_offset_and_spritesheet.x, r.sprite_offset_and_spritesheet.y };
  const glm::vec2 spritesheet = { r.sprite_offset_and_spritesheet.z, r.sprite_offset_and_spritesheet.w };
  const float tex_unit = static_cast<float>(r.tex_unit);
  const glm::vec4 colour = { r.colour.r, r.colour.g, r.colour.b, r.colour.a };

  // tl
  data.buffer_ptr->pos_and_uv = { 0.0f, 0.0f, 0.0f, 0.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset_and_spritesheet = { sprite_offset, spritesheet };
  data.buffer_ptr->tex_unit = tex_unit;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  // tr
  data.buffer_ptr->pos_and_uv = { 1.0f, 0.0f, 1.0f, 0.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset_and_spritesheet = { sprite_offset, spritesheet };
  data.buffer_ptr->tex_unit = tex_unit;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  // br
  data.buffer_ptr->pos_and_uv = { 1.0f, 1.0f, 1.0f, 1.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset_and_spritesheet = { sprite_offset, spritesheet };
  data.buffer_ptr->tex_unit = tex_unit;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  // bl
  data.buffer_ptr->pos_and_uv = { 0.0f, 1.0f, 0.0f, 1.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset_and_spritesheet = { sprite_offset, spritesheet };
  data.buffer_ptr->tex_unit = tex_unit;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.index_count += 6;
  data.quad_vertex += 4;
}

void
QuadRenderer::init()
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
  glVertexAttribPointer(
    2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, sprite_offset_and_spritesheet));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_unit));

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
  uint32_t index_offset = 0;
  for (int i = 0; i < max_quad_index_count; i += 6) {
    indices[i + 0] = 0 + index_offset;
    indices[i + 1] = 1 + index_offset;
    indices[i + 2] = 2 + index_offset;

    indices[i + 3] = 2 + index_offset;
    indices[i + 4] = 3 + index_offset;
    indices[i + 5] = 0 + index_offset;

    index_offset += 4;
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // unbind vbo and vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
};

void
QuadRenderer::shutdown()
{
  glDeleteVertexArrays(1, &data.VAO);
  glDeleteBuffers(1, &data.VBO);
  glDeleteBuffers(1, &data.EBO);

  delete[] data.buffer;
}

// upload data to vbo
void
QuadRenderer::end_batch()
{
  GLsizeiptr size = (uint8_t*)data.buffer_ptr - (uint8_t*)data.buffer;
  // Set dynamic vertex buffer & upload data
  glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
  // glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data.buffer);
}

void
QuadRenderer::flush(const Shader& shader)
{
  shader.bind();

  if (data.index_count > 0) {
    glBindVertexArray(data.VAO);
    glDrawElements(GL_TRIANGLES, data.index_count, GL_UNSIGNED_INT, nullptr);
    // glDrawElementsInstanced(GL_TRIANGLES, data.index_count, GL_UNSIGNED_INT, 0, 1000);
    data.draw_calls += 1;
    data.index_count = 0;
  }

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void
QuadRenderer::begin_batch()
{
  data.buffer_ptr = data.buffer;
}

void
QuadRenderer::reset_quad_vert_count()
{
  data.quad_vertex = 0;
}

void
QuadRenderer::end_frame()
{
  data.draw_calls = 0;
}

int
QuadRenderer::draw_calls()
{
  return data.draw_calls;
}

} // namespace quad_renderer

} // namespace engine