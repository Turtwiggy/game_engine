
// header
#include "batch_sprite.hpp"

// engine project headers
#include "helpers/renderers/batch_renderer.hpp"

// other project headers
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {

namespace sprite_renderer {

static RenderData<Vertex> data;

void
SpriteBatchRenderer::draw_sprite(const RenderDescriptor& r, Shader& s)
{
  if (data.index_count >= max_quad_index_count) {
    end_batch();
    flush(s);
    begin_batch();
  }

  const float& angle = r.angle_radians;
  const glm::vec2& pos = r.pos_tl;
  const glm::vec2& size = r.size;
  const glm::ivec2& sprite_offset = r.sprite_offset;
  const float& tex_slot = r.tex_slot;
  const glm::vec4& colour = r.colour;

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(pos.x, pos.y, 0.0f));
  model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
  model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
  model = glm::scale(model, glm::vec3(size, 1.0f));

  // tl
  data.buffer_ptr->pos_and_uv = { 0.0f, 0.0f, 0.0f, 0.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->tex_slot = tex_slot;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  // tr
  data.buffer_ptr->pos_and_uv = { 1.0f, 0.0f, 1.0f, 0.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->tex_slot = tex_slot;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  // br
  data.buffer_ptr->pos_and_uv = { 1.0f, 1.0f, 1.0f, 1.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->tex_slot = tex_slot;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  // bl
  data.buffer_ptr->pos_and_uv = { 0.0f, 1.0f, 0.0f, 1.0f };
  data.buffer_ptr->colour = colour;
  data.buffer_ptr->sprite_offset = sprite_offset;
  data.buffer_ptr->tex_slot = tex_slot;
  data.buffer_ptr->model = model;
  data.buffer_ptr++;

  data.index_count += 6;
  data.quad_vertex += 4;
}

void
SpriteBatchRenderer::init()
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
SpriteBatchRenderer::shutdown()
{
  glDeleteVertexArrays(1, &data.VAO);
  glDeleteBuffers(1, &data.VBO);
  glDeleteBuffers(1, &data.EBO);

  delete[] data.buffer;
}

// upload data to vbo
void
SpriteBatchRenderer::end_batch()
{
  GLsizeiptr size = (uint8_t*)data.buffer_ptr - (uint8_t*)data.buffer;
  // Set dynamic vertex buffer & upload data
  glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
  // glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data.buffer);
}

void
SpriteBatchRenderer::flush(Shader& shader)
{
  shader.bind();

  glBindVertexArray(data.VAO);
  glDrawElements(GL_TRIANGLES, data.index_count, GL_UNSIGNED_INT, nullptr);

  data.draw_calls += 1;
  data.index_count = 0;

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void
SpriteBatchRenderer::begin_batch()
{
  data.buffer_ptr = data.buffer;
}

void
SpriteBatchRenderer::reset_quad_vert_count()
{
  data.quad_vertex = 0;
}

void
SpriteBatchRenderer::end_frame()
{
  data.draw_calls = 0;
}

// void
// sprite_renderer::draw_sprites_debug(const RenderDescriptor& cam,
//                                     const glm::ivec2& screen_size,
//                                     const std::vector<std::reference_wrapper<RenderDescriptor>>& objects,
//                                     Shader& debug_line_shader,
//                                     const glm::vec4& debug_line_shader_colour)
// {
// #ifdef WIN32
// #ifdef _DEBUG
//   // draw lines
//   debug_line_shader.bind();
//   debug_line_shader.set_vec4("colour", debug_line_shader_colour);

//   // This API is deprecated, does not work on phones/tablets.
//   for (auto& obj : objects) {
//     glBegin(GL_LINE_LOOP);
//     glm::vec2 world_pos = gameobject_in_worldspace(cam, obj.get());
//     glm::vec2 bl_pos = glm::vec2(world_pos.x, world_pos.y + obj.get().physics_size.y);
//     glm::vec2 tr_pos = glm::vec2(world_pos.x + obj.get().physics_size.x, world_pos.y);
//     bl_pos.x = scale(bl_pos.x, 0.0f, screen_size.x, -1.0f, 1.0f);
//     bl_pos.y = scale(bl_pos.y, 0.0f, screen_size.y, 1.0f, -1.0f);
//     tr_pos.x = scale(tr_pos.x, 0.0f, screen_size.x, -1.0f, 1.0f);
//     tr_pos.y = scale(tr_pos.y, 0.0f, screen_size.y, 1.0f, -1.0f);
//     glVertex2f(bl_pos.x, bl_pos.y);
//     glVertex2f(tr_pos.x, bl_pos.y);
//     glVertex2f(tr_pos.x, tr_pos.y);
//     glVertex2f(bl_pos.x, tr_pos.y);
//     glEnd();
//   }
// #endif
// #endif
// }

} // namespace sprite_renderer

} // namespace engine