#pragma once

#include <glm/glm.hpp>

namespace game2d {

#define TILE_PIX 16
#define TILE_SEP 1

struct Camera2D
{
  glm::vec2 pos;
  float zoom_level = 0;

  // glm::vec2 tile_to_screen(render_window& win, glm::vec2 tile_pos) const;
  // glm::vec2 screen_to_tile(render_window& win, glm::vec2 screen_pos) const;
  // glm::vec2 world_to_screen(render_window& win, glm::vec2 world_pos) const;
  // glm::vec2 screen_to_world(render_window& win, glm::vec2 screen_pos) const;
  static glm::vec2 tile_to_world(glm::vec2 pos);
  // static glm::vec2 world_to_tile(glm::vec2 pos);

  void translate(glm::vec2 amount);

  void zoom(float number_of_levels);

  float calculate_scale() const;
};

} // namespace game2d