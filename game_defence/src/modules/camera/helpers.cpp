#include "helpers.hpp"

#include "events/helpers/mouse.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "orthographic.hpp"

#include "entt/helpers.hpp"

namespace game2d {

glm::ivec2
mouse_position_in_worldspace(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& cameras = r.view<OrthographicCamera, TransformComponent>(entt::exclude<WaitForInitComponent>);
  const auto mouse_pos = get_mouse_pos() - ri.viewport_pos;

  glm::ivec2 camera_position;
  for (auto [entity, camera, transform] : cameras.each())
    camera_position = { transform.position.x, transform.position.y };

  const glm::ivec2 xy{
    //
    ri.viewport_size_render_at.x / 2.0f,
    ri.viewport_size_render_at.y / 2.0f
    //
  };

  const glm::ivec2 pos_in_worldspace = {
    camera_position.x + mouse_pos.x - xy.x,
    camera_position.y + mouse_pos.y - xy.y,
  };

  return pos_in_worldspace;
};

glm::ivec2
position_in_worldspace(entt::registry& r, const glm::ivec2& game_pos)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& cameras = r.view<OrthographicCamera, TransformComponent>(entt::exclude<WaitForInitComponent>);

  const auto screen_pos = game_pos - ri.viewport_pos;

  glm::ivec2 camera_position;
  for (auto [entity, camera, transform] : cameras.each())
    camera_position = { transform.position.x, transform.position.y };

  const glm::ivec2 xy{
    //
    ri.viewport_size_render_at.x / 2.0f,
    ri.viewport_size_render_at.y / 2.0f
    //
  };

  const glm::ivec2 pos_in_worldspace = {
    screen_pos.x + xy.x - camera_position.x,
    screen_pos.y + xy.y - camera_position.y,
  };
  return pos_in_worldspace;
};

} // namespace game2d