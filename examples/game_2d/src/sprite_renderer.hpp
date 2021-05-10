#pragma once

// other project headers
#include <glm/glm.hpp>

// your project headers
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

struct GameObject2D
{
  //
  // metadata
  // note: not currently thread safe.
  //
  static inline int global_int_counter = 0;
  int id = 0;
  std::string name = "DEFAULT";

  //
  // rendering
  //
  sprite::type sprite = sprite::type::SQUARE;
  int tex_slot = 0;

  //
  // physics
  //
  CollisionLayer collision_layer = CollisionLayer::Default;

  // todo: interpolation
  bool is_bullet = false;
  glm::vec2 pos_last_frame = { 0.0f, 0.0f };
  float distance_before_needing_interpolate_point = 10.0f;

  //
  // game
  //
  glm::vec2 pos = { 0.0f, 0.0f }; // in pixels, centered
  float angle_radians = 0.0f;
  glm::vec2 size = { 20.0f, 20.0f };
  glm::vec4 colour = { 1.0f, 0.0f, 0.0f, 1.0f };
  glm::vec2 velocity = { 0.0f, 0.0f };
  float velocity_boost_modifier = 2.0f;

  // lifecycle
  float time_alive_left = 5.0f;

  // hittable
  int hits_able_to_be_taken = 30;
  int hits_taken = 0;
  bool invulnerable = false;

  // speed
  float speed_current = 50.0f;
  float speed_default = 50.0f;

  GameObject2D() { id = ++GameObject2D::global_int_counter; }
};

namespace sprite_renderer {

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
void
render_quad();

void
draw_sprite(GameObject2D& cam,
            const glm::ivec2& screen_size,
            fightingengine::Shader& shader,
            glm::vec2 position,
            glm::vec2 size,
            float angle,
            glm::vec3 color,
            int tex_slot = 0);

void
draw_sprite_debug(GameObject2D& cam,
                  const glm::ivec2& screen_size,
                  fightingengine::Shader& shader,
                  GameObject2D& game_object,
                  fightingengine::Shader& debug_line_shader,
                  glm::vec4& debug_line_shader_colour,
                  int tex_slot = 0);

} // namespace sprite_renderer

} // namespace game2d