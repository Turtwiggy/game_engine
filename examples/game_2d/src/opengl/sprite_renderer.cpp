
// header
#include "opengl/sprite_renderer.hpp"

// standard lib headers
#include <array>
#include <iostream>

// other project headers
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

// engine project headers
#include "engine/maths_core.hpp"
#include "engine/opengl/util.hpp"
using namespace fightingengine; // used for opengl macro

// game headers
#include "spritemap.hpp"

namespace game2d {

namespace sprite_renderer {

//
// V2 Renderer (Dynamic Batched Draw Calls)
//

struct Vertex
{
  glm::vec4 pos_and_tex;
  glm::vec4 colour;
  glm::vec2 sprite_pos;
  float tex_index;
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

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, pos_and_tex));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, colour));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, sprite_pos));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_index));

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

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_data.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // unbind vbo and vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}
void
shutdown()
{
  glDeleteVertexArrays(1, &s_data.VAO);
  glDeleteBuffers(1, &s_data.VBO);
  glDeleteBuffers(1, &s_data.EBO);

  delete[] s_data.buffer;
}

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
  glDrawElements(GL_TRIANGLES, s_data.index_count, GL_UNSIGNED_INT, nullptr);

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
draw_instanced_sprite(const GameObject2D& cam,
                      const glm::ivec2& screen_size,
                      fightingengine::Shader& shader,
                      const GameObject2D& go)
{
  draw_instanced_sprite(cam, screen_size, shader, go, go.colour, go.colour, go.colour, go.colour);
}

void
draw_instanced_sprite(const GameObject2D& cam,
                      const glm::ivec2& screen_size,
                      fightingengine::Shader& shader,
                      const GameObject2D& go,
                      const glm::vec4 colour_tl,
                      const glm::vec4 colour_tr,
                      const glm::vec4 colour_bl,
                      const glm::vec4 colour_br)
{
  if (s_data.index_count >= max_quad_index_count) {
    end_batch();
    flush(shader);
    begin_batch();
  }

  glm::vec2 worldspace_pos = gameobject_in_worldspace(cam, go);
  if (gameobject_off_screen(worldspace_pos, go.render_size, screen_size)) {
    return; // skip rendering
  }

  glm::ivec2 sprite_offset = sprite::spritemap::get_sprite_offset(go.sprite);
  float angle = go.angle_radians + sprite::spritemap::get_sprite_rotation_offset(go.sprite);

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(glm::vec2(worldspace_pos.x, worldspace_pos.y), 0.0f));
  model = glm::translate(model, glm::vec3(0.5f * go.render_size.x, 0.5f * go.render_size.y, 0.0f));
  model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::translate(model, glm::vec3(-0.5f * go.render_size.x, -0.5f * go.render_size.y, 0.0f));
  model = glm::scale(model, glm::vec3(go.render_size, 1.0f));

  // tl
  s_data.buffer_ptr->pos_and_tex = { 0.0f, 0.0f, 0.0f, 0.0f };
  s_data.buffer_ptr->colour = colour_tl;
  s_data.buffer_ptr->sprite_pos = sprite_offset;
  s_data.buffer_ptr->tex_index = (float)go.tex_slot;
  s_data.buffer_ptr->model = model;
  s_data.buffer_ptr++;

  // tr
  s_data.buffer_ptr->pos_and_tex = { 1.0f, 0.0f, 1.0f, 0.0f };
  s_data.buffer_ptr->colour = colour_tr;
  s_data.buffer_ptr->sprite_pos = sprite_offset;
  s_data.buffer_ptr->tex_index = (float)go.tex_slot;
  s_data.buffer_ptr->model = model;
  s_data.buffer_ptr++;

  // br
  s_data.buffer_ptr->pos_and_tex = { 1.0f, 1.0f, 1.0f, 1.0f };
  s_data.buffer_ptr->colour = colour_br;
  s_data.buffer_ptr->sprite_pos = sprite_offset;
  s_data.buffer_ptr->tex_index = (float)go.tex_slot;
  s_data.buffer_ptr->model = model;
  s_data.buffer_ptr++;

  // bl
  s_data.buffer_ptr->pos_and_tex = { 0.0f, 1.0f, 0.0f, 1.0f };
  s_data.buffer_ptr->colour = colour_bl;
  s_data.buffer_ptr->sprite_pos = sprite_offset;
  s_data.buffer_ptr->tex_index = (float)go.tex_slot;
  s_data.buffer_ptr->model = model;
  s_data.buffer_ptr++;

  s_data.index_count += 6;
  s_data.quad_vertex += 4;
}

void
draw_sprites_debug(const GameObject2D& cam,
                   const glm::ivec2& screen_size,
                   const std::vector<std::reference_wrapper<GameObject2D>>& game_objects,
                   fightingengine::Shader& debug_line_shader,
                   const glm::vec4& debug_line_shader_colour)
{
#ifdef WIN32
#ifdef _DEBUG
  // draw lines
  debug_line_shader.bind();
  debug_line_shader.set_vec4("colour", debug_line_shader_colour);

  // This API is deprecated, does not work on phones/tablets.
  for (auto& obj : game_objects) {
    glBegin(GL_LINE_LOOP);
    glm::vec2 world_pos = gameobject_in_worldspace(cam, obj.get());
    glm::vec2 bl_pos = glm::vec2(world_pos.x, world_pos.y + obj.get().physics_size.y);
    glm::vec2 tr_pos = glm::vec2(world_pos.x + obj.get().physics_size.x, world_pos.y);
    bl_pos.x = fightingengine::scale(bl_pos.x, 0.0f, screen_size.x, -1.0f, 1.0f);
    bl_pos.y = fightingengine::scale(bl_pos.y, 0.0f, screen_size.y, 1.0f, -1.0f);
    tr_pos.x = fightingengine::scale(tr_pos.x, 0.0f, screen_size.x, -1.0f, 1.0f);
    tr_pos.y = fightingengine::scale(tr_pos.y, 0.0f, screen_size.y, 1.0f, -1.0f);
    glVertex2f(bl_pos.x, bl_pos.y);
    glVertex2f(tr_pos.x, bl_pos.y);
    glVertex2f(tr_pos.x, tr_pos.y);
    glVertex2f(bl_pos.x, tr_pos.y);
    glEnd();
  }
#endif
#endif
}

} // namespace sprite_renderer

} // namespace game2d