#include "helpers.hpp"

namespace game2d {

glm::ivec2
mouse_position_in_worldspace(GameEditor& editor, Game& game)
{
  const auto& ri = editor.renderer;
  auto& registry = game.state;

  const auto& cameras = registry.view<CameraComponent, TransformComponent>();

  glm::ivec2 camera_position;
  for (auto [entity, camera, transform] : cameras.each())
    camera_position = { transform.position.x, transform.position.y };

  const auto mouse_pos = get_mouse_pos() - ri.viewport_pos;

  const glm::ivec2 mouse_pos_in_worldpsace = {
    mouse_pos.x + (camera_position.x),
    mouse_pos.y + (camera_position.y),
  };
  return mouse_pos_in_worldpsace;
};

} // namespace game2d