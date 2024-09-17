#include "helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/components.hpp"
#include "orthographic.hpp"

namespace game2d {

glm::vec2
mouse_position_in_worldspace(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_t = r.get<const TransformComponent>(camera_e);

  const glm::vec2 mouse_pos = get_mouse_pos() - ri.viewport_pos;
  const glm::vec2 camera_pos = { camera_t.position.x, camera_t.position.y };

  return mouse_pos - camera_pos;
};

glm::ivec2
position_in_worldspace(entt::registry& r, const glm::ivec2& game_pos)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto& cameras = r.view<OrthographicCamera, TransformComponent>(entt::exclude<WaitForInitComponent>);

  const auto screen_pos = game_pos - ri.viewport_pos;

  glm::vec2 camera_position{ 0.0f, 0.0f };
  for (const auto& [entity, camera, transform] : cameras.each())
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