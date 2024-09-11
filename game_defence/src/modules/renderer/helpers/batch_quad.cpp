
// header
#include "batch_quad.hpp"

// my libs
#include "modules/vfx_circle/components.hpp"

// other project headers
#include "deps/opengl.hpp"
#include "opengl/util.hpp"

#include <fmt/core.h>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {

namespace quad_renderer {
using game2d::CircleComponent;

void
QuadRenderer::draw_sprite(const RenderDescriptor& r, const Shader& s)
{
  if (data.index_count >= max_quad_index_count) {
    end_batch();
    flush(s);
    begin_batch();
  }

  const glm::vec4 sprite_offset = { r.sprite_offset.x, r.sprite_offset.y, 0.0f, 0.0f };
  const glm::vec4 sprite_width_and_max = { r.sprite_width.x, r.sprite_width.y, r.sprites_max.x, r.sprites_max.y };
  const float tex_unit = static_cast<float>(r.tex_unit);
  const glm::vec4 colour = { r.colour.r, r.colour.g, r.colour.b, r.colour.a };

  const float& angle = r.angle_radians;
  const glm::vec2& pos = r.pos_tl;
  const glm::vec2& size = r.size;
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(pos.x, pos.y, 0.0f));
  model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
  if (angle != 0.0f)
    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
  model = glm::scale(model, glm::vec3(size, 1.0f));

  // tl
  data.buffer_ptr->pos_and_uv = { 0.0f, 0.0f, 0.0f, 0.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->sprite_width_and_max = sprite_width_and_max;
  data.buffer_ptr->tex_unit = tex_unit;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  // tr
  data.buffer_ptr->pos_and_uv = { 1.0f, 0.0f, 1.0f, 0.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->sprite_width_and_max = sprite_width_and_max;
  data.buffer_ptr->tex_unit = tex_unit;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  // br
  data.buffer_ptr->pos_and_uv = { 1.0f, 1.0f, 1.0f, 1.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->sprite_width_and_max = sprite_width_and_max;
  data.buffer_ptr->tex_unit = tex_unit;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  // bl
  data.buffer_ptr->pos_and_uv = { 0.0f, 1.0f, 0.0f, 1.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->sprite_width_and_max = sprite_width_and_max;
  data.buffer_ptr->tex_unit = tex_unit;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.index_count += 6;
  data.quad_vertex += 4;
}

void
QuadRenderer::init()
{
  CHECK_OPENGL_ERROR(4); // eat any error before this stage

  data.buffer = new Vertex[max_quad_vert_count];

  glGenVertexArrays(1, &data.VAO);
  glGenBuffers(1, &data.VBO);
  glGenBuffers(1, &data.EBO);

  glBindVertexArray(data.VAO); // bind the vao

  glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
  glBufferData(GL_ARRAY_BUFFER, max_quad_vert_count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW); // dynamic

  // specific to the game...

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, pos_and_uv));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, colour));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, sprite_offset));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, sprite_width_and_max));

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_unit));

  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, model));

  glEnableVertexAttribArray(6);
  glVertexAttribPointer(
    6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, model) + 1 * sizeof(glm::vec4)));

  glEnableVertexAttribArray(7);
  glVertexAttribPointer(
    7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, model) + 2 * sizeof(glm::vec4)));

  glEnableVertexAttribArray(8);
  glVertexAttribPointer(
    8, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, model) + 3 * sizeof(glm::vec4)));

  uint32_t indices[max_quad_index_count];
  uint32_t index_offset = 0;
  for (uint32_t i = 0; i < max_quad_index_count; i += 6) {
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

  // create a texture buffer object
  GLuint& tbo = data.TBO;
  GLuint& tex = data.TEX;
  glGenBuffers(1, &tbo);
  glBindBuffer(GL_TEXTURE_BUFFER, tbo);
  glBufferData(GL_TEXTURE_BUFFER, N_MAX_CIRCLES * sizeof(CircleComponent), nullptr, GL_DYNAMIC_DRAW);

  // bind the tbo to a texture unit for shaders
  glGenTextures(1, &tex);

  glBindTexture(GL_TEXTURE_BUFFER, tex);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, tbo);

  fmt::println("created texture object...");
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