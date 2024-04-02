#include "helpers.hpp"

#include "events/helpers/mouse.hpp"
#include "orthographic.hpp"
#include "modules/renderer/components.hpp"

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

  const glm::ivec2 xy{
    //
    ri.viewport_size_render_at.x / 2.0f,
    ri.viewport_size_render_at.y / 2.0f
    //
  };

  const glm::ivec2 mouse_pos_in_worldspace = {
    camera_position.x + mouse_pos.x - xy.x,
    camera_position.y + mouse_pos.y - xy.y,
  };
  return mouse_pos_in_worldspace;
};

} // namespace game2d