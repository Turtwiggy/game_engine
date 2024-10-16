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
  const auto& wh = ri.viewport_size_render_at;
  const auto half_wh = glm::vec2{ wh.x * 0.5f, wh.y * 0.5f };

  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_c = r.get<OrthographicCamera>(camera_e);
  const auto& camera_t = r.get<const TransformComponent>(camera_e);

  // mouse position relative to the top-left of the window
  const glm::vec2 mouse_pos = get_mouse_pos() - ri.viewport_pos;

  const auto camera_pos = glm::vec2{ camera_t.position.x, camera_t.position.y };
  const auto zoom = camera_c.zoom_nonlinear;

  const float mouse_offset_x = mouse_pos.x - half_wh.x;
  const float mouse_offset_y = mouse_pos.y - half_wh.y;

  const glm::vec2 pos_in_worldspace = {
    camera_pos.x + (mouse_offset_x * zoom),
    camera_pos.y + (mouse_offset_y * zoom),
  };

  return pos_in_worldspace;
};

glm::vec2
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

  const glm::vec2 pos_in_worldspace = {
    screen_pos.x + xy.x - camera_position.x,
    screen_pos.y + xy.y - camera_position.y,
  };
  return pos_in_worldspace;
};

} // namespace game2d