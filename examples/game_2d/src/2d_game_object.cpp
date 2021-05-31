// your header
#include "2d_game_object.hpp"

namespace game2d {

glm::vec2
gameobject_in_worldspace(const GameObject2D& camera, const GameObject2D& go)
{
  return go.pos - camera.pos;
}

bool
gameobject_off_screen(glm::vec2 pos, glm::vec2 size, const glm::ivec2& screen_size)
{
  glm::vec2 tl_visible = glm::vec2(0.0f, 0.0f);
  glm::vec2 br_visible = screen_size;
  if (
    // left of screen
    pos.x + size.x < tl_visible.x ||
    // top of screen
    pos.y + size.y < tl_visible.y ||
    // right of screen
    pos.x > br_visible.x ||
    // bottom of screen
    pos.y > br_visible.y) {
    return true;
  }
  return false;
}

} // namespace game2d