#pragma once

// other project headers
#include <glm/glm.hpp>

// your project headers
#include "camera2d.hpp"
#include "engine/opengl/shader.hpp"
#include "spritemap.hpp"

namespace game2d {

enum class CollisionLayer
{
  Default = 0,
  Player = 1,
  Bullet = 2,
  Destroyable = 3,
};

struct Collision2D
{
  int obj_id_1;
  int obj_id_2;
};

struct GameObject2D
{
  static inline int global_int_counter = 0;
  int id = 0;
  std::string name = "DEFAULT";

  glm::vec2 pos = { 0.0f, 0.0f }; // in pixels, centered
  float angle_radians = 0.0f;
  glm::vec2 size = { 20.0f, 20.0f };
  glm::vec4 colour = { 1.0f, 0.0f, 0.0f, 1.0f };
  glm::vec2 velocity = { 0.0f, 0.0f };

  sprite::type sprite = sprite::type::SQUARE;
  CollisionLayer collision_layer = CollisionLayer::Default;
  int tex_slot = 0;

  GameObject2D()
  {
    global_int_counter++;
    id = global_int_counter;
  }
};

namespace sprite_renderer {

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
void
render_quad();

void
draw_sprite(Camera2D& cam,
            const glm::ivec2& screen_size,
            fightingengine::Shader& shader,
            glm::vec2 position,
            glm::vec2 size,
            float angle,
            glm::vec3 color,
            int tex_slot = 0);

void
draw_sprite_debug(Camera2D& cam,
                  const glm::ivec2& screen_size,
                  fightingengine::Shader& shader,
                  GameObject2D& game_object,
                  fightingengine::Shader& debug_line_shader,
                  glm::vec4& debug_line_shader_colour,
                  int tex_slot = 0);

} // namespace sprite_renderer

} // namespace game2d