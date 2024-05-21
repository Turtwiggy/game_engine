// header
#include "system.hpp"

// components/systems
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/components.hpp"
#include "renderer/transform.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

namespace game2d {

void
update_camera_system(entt::registry& r, const float dt, const glm::ivec2& mouse_pos)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto camera_ent = get_first<OrthographicCamera>(r);

  const float screen_x = static_cast<int>(-ri.viewport_size_render_at.x / 2.0f);
  const float screen_y = static_cast<int>(-ri.viewport_size_render_at.y / 2.0f);

  auto& camera = r.get<OrthographicCamera>(camera_ent);
  auto& camera_transform = r.get<TransformComponent>(camera_ent);

  // calculate direction mouse is in, and add that to thes camera
  // nb: mouse_pos is in worldspace, not screenspace.
  // glm::vec2 camera_offset_due_to_mouse{ 0, 0 };
  // const glm::vec2 raw_dir = camera_position_worldspace - mouse_pos;
  // camera_offset_due_to_mouse.x = dir.x * 10.0f;
  // camera_offset_due_to_mouse.y = dir.y * 10.0f;

  // Set position as first position of target
  const auto& targets_view = r.view<const CameraFollow>();
  for (const auto& [e, follow] : targets_view.each()) {
    const auto pos = get_position(r, e);
    camera_transform.position.x = pos.x;
    camera_transform.position.y = pos.y;
  }

  // if (targets_view.size_hint() > 0) {
  //   target_position.x /= targets_view.size_hint();
  //   target_position.y /= targets_view.size_hint();
  // }

  // update lerp
  // if (auto* target = r.try_get<HasTargetPositionComponent>(camera_ent))
  //   target->position = { target_position.x, target_position.y };

  // allow move camera if no targets
  if (targets_view.size() == 0) {
    const float CAM_SPEED = 500.0f;
    const int mul = static_cast<int>(CAM_SPEED * dt);
    glm::vec2 movement{ 0, 0 };
    if (get_key_held(input, SDL_SCANCODE_A))
      movement.x -= mul;
    if (get_key_held(input, SDL_SCANCODE_D))
      movement.x += mul;
    if (get_key_held(input, SDL_SCANCODE_W))
      movement.y -= mul;
    if (get_key_held(input, SDL_SCANCODE_S))
      movement.y += mul;

    // if (get_mouse_mmb_held()) { // pan
    // if (get_mouse_lmb_held()) { // rotate
    // if (get_mouse_rmb_held()) { // zoom

    // update the transform
    camera_transform.position.x += movement.x;
    camera_transform.position.y += movement.y;
    // camera_transform.position.z;
  }

  // calculate view after updating postiion
  TransformComponent screen_offset = camera_transform;
  screen_offset.position.x = screen_x + screen_offset.position.x;
  screen_offset.position.y = screen_y + screen_offset.position.y;
  camera.view = calculate_ortho_view(screen_offset);
};

} // namespace game2d