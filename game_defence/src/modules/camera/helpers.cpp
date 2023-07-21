#include "helpers.hpp"

#include "events/helpers/mouse.hpp"
#include "orthographic.hpp"
#include "renderer/components.hpp"

#include "entt/helpers.hpp"

namespace game2d {

glm::ivec2
mouse_position_in_worldspace(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& cameras = r.view<OrthographicCamera, TransformComponent>();

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