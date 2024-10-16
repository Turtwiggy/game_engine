// header
#include "system.hpp"

// components/systems
#include "actors/actor_helpers.hpp"
#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "orthographic.hpp"

#include <cmath>
#include <glm/glm.hpp>

namespace game2d {

void
update_camera_system(entt::registry& r, const float dt)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto& input = get_first_component<SINGLE_InputComponent>(r);
  const auto camera_ent = get_first<OrthographicCamera>(r);

  const float screen_x = -ri.viewport_size_render_at.x / 2.0f;
  const float screen_y = -ri.viewport_size_render_at.y / 2.0f;
  auto& camera = r.get<OrthographicCamera>(camera_ent);
  auto& camera_transform = r.get<TransformComponent>(camera_ent);

  // calculate direction mouse is in, and add that to thes camera
  // nb: mouse_pos is in worldspace, not screenspace.
  // glm::vec2 camera_offset_due_to_mouse{ 0, 0 };
  // const glm::vec2 raw_dir = camera_position_worldspace - mouse_pos;
  // camera_offset_due_to_mouse.x = dir.x * 10.0f;
  // camera_offset_due_to_mouse.y = dir.y * 10.0f;

  // Set position as first position of target
  const auto& targets_view = r.view<const CameraFollow, const TransformComponent>();
  for (const auto& [e, follow, t_c] : targets_view.each()) {
    camera_transform.position.x = t_c.position.x;
    camera_transform.position.y = t_c.position.y;
  }

  // update lerp
  auto target_e = get_first<CameraLerpToTarget>(r);
  if (target_e != entt::null) {
    auto& lerp_c = r.get_or_emplace<LerpToMovingTarget>(camera_ent);
    lerp_c.a = get_position(r, camera_ent);
    lerp_c.b = get_position(r, target_e);
    lerp_c.speed = 5.0f;
  } else
    remove_if_exists<LerpToMovingTarget>(r, camera_ent);

  // allow camera to move freely
  if (get_first<CameraFreeMove>(r) != entt::null) {
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
  camera.view = calculate_ortho_view(screen_offset, dt);

  // no zooming unless on the viewport
  if (!ri.viewport_hovered)
    return;

  auto& zoom = camera.zoom_linear;
  auto& zoom_nonlinear = camera.zoom_nonlinear;

  if (ImGui::GetIO().MouseWheel > 0.0f)
    zoom -= 100.0f * dt;
  if (ImGui::GetIO().MouseWheel < 0.0f)
    zoom += 100.0f * dt;

  // If zoom = 0, then 2^(zoom / 2) gives you a zoom factor of 1 (no zoom).
  // If zoom = 1, then 2^(1 / 2) gives a zoom factor of ~1.414 (approximately zooming in by 41%).
  // If zoom = -1, then 2^(-1 / 2) gives a zoom factor of ~0.707 (zooming out by 29%).
  float new_zoom_nonlinear = glm::pow(2.0f, (zoom / 2.0f));
  const float speed = 7.5;
  const float zoom_in = 0.25f;
  const float zoom_out = 2.0f;
  zoom_nonlinear = engine::lerp(zoom_nonlinear, new_zoom_nonlinear, dt * speed);

  // clamp zoomout
  if (zoom_nonlinear > zoom_out) {
    zoom_nonlinear = zoom_out;
    zoom = (2.0f * std::log(zoom_nonlinear)) / std::log(2.0f);
  };

  // clamp zoomin
  if (zoom_nonlinear < zoom_in) {
    zoom_nonlinear = zoom_in;
    zoom = (2.0f * std::log(zoom_nonlinear)) / std::log(2.0f);
  }

  camera.projection_zoomed =
    calculate_ortho_projection(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y, zoom_nonlinear);
};

} // namespace game2d