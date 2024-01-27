// header
#include "system.hpp"

// components/systems
#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/renderer/components.hpp"
#include "renderer/transform.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

#include "imgui.h"

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
  auto& camera_position_worldspace = camera.base_position;

  // calculate direction mouse is in, and add that to thes camera
  // nb: mouse_pos is in worldspace, not screenspace.
  glm::vec2 camera_offset_due_to_mouse{ 0, 0 };
  // glm::vec2 dir = camera_position_worldspace - mouse_pos;
  // if (dir.x != 0.0f || dir.y != 0.0f)
  //   dir = glm::normalize(dir);
  // camera_offset_due_to_mouse.x = dir.x * 10.0f;
  // camera_offset_due_to_mouse.y = dir.y * 10.0f;

  // calculate center of all targets
  // auto target_position = camera_transform.position;
  // const auto& targets_view = r.view<CameraFollow, TransformComponent>();
  // for (const auto& [entity, follow, target_transform] : targets_view.each()) {
  //   target_position.x += target_transform.position.x;
  //   target_position.x /= 2.0f;
  //   target_position.y += target_transform.position.y;
  //   target_position.y /= 2.0f;
  // }

  // update lerp
  // if (auto* target = r.try_get<HasTargetPositionComponent>(camera_ent))
  //   target->position = { target_position.x, target_position.y };

  const float CAM_SPEED = 500.0f;
  const int mul = static_cast<int>(CAM_SPEED * dt);
  if (get_key_held(input, SDL_SCANCODE_A))
    camera_position_worldspace.x -= mul;
  if (get_key_held(input, SDL_SCANCODE_D))
    camera_position_worldspace.x += mul;
  if (get_key_held(input, SDL_SCANCODE_W))
    camera_position_worldspace.y -= mul;
  if (get_key_held(input, SDL_SCANCODE_S))
    camera_position_worldspace.y += mul;

  // if (get_mouse_mmb_held()) { // pan
  // if (get_mouse_lmb_held()) { // rotate
  // if (get_mouse_rmb_held()) { // zoom

  // update the transform
  auto& camera_transform = r.get<TransformComponent>(camera_ent);
  camera_transform.position.x = camera_position_worldspace.x + camera_offset_due_to_mouse.x;
  camera_transform.position.y = camera_position_worldspace.y + camera_offset_due_to_mouse.y;
  camera_transform.position.z;

  // calculate view after updating postiion
  TransformComponent screen_offset = camera_transform;
  screen_offset.position.x = screen_x + screen_offset.position.x;
  screen_offset.position.y = screen_y + screen_offset.position.y;
  camera.view = calculate_ortho_view(screen_offset);
};

} // namespace game2d