#include "pch.hpp"

// header
#include "camera_system.hpp"

// components/systems
#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/systems/system_screenshake/components.hpp"

namespace game2d {

// https://www.youtube.com/watch?v=LSNQuFEDOyQ
const auto exp_decay = [](float a, float b, float decay, float dt) -> float {
  //
  return b + (a - b) * glm::exp(-decay * dt);
  //
};

void
update_zoom(OrthographicCamera& camera, float dt)
{
  auto& zoom = camera.zoom_linear;
  auto& zoom_nonlinear = camera.zoom_nonlinear;

  const float speed = 15.0;
  const float zoom_in = 0.25f;
  const float zoom_out = 2.0f;

  // If zoom = 0, then 2^(zoom / 2) gives you a zoom factor of 1 (no zoom).
  // If zoom = 1, then 2^(1 / 2) gives a zoom factor of ~1.414 (approximately zooming in by 41%).
  // If zoom = -1, then 2^(-1 / 2) gives a zoom factor of ~0.707 (zooming out by 29%).
  float new_zoom_nonlinear = glm::pow(2.0f, (zoom / 2.0f));

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

  // ImGui::SeparatorText("DebugCamera");
  // ImGui::Text("zoom: %f", zoom);
  // ImGui::Text("zoom nonlinear: %f", zoom_nonlinear);
};

void
update_camera_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto& ri = SINGLE_RendererInfo::instance;
  const auto& input = SINGLE_InputComponent::instance;
  const auto camera_ent = get_first<OrthographicCamera>(r);

  auto& camera = r.get<OrthographicCamera>(camera_ent);
  auto& camera_transform = r.get<TransformComponent>(camera_ent);

  // calculate direction mouse is in, and add that to thes camera
  // nb: mouse_pos is in worldspace, not screenspace.
  // glm::vec2 camera_offset_due_to_mouse{ 0, 0 };
  // const glm::vec2 raw_dir = camera_position_worldspace - mouse_pos;
  // camera_offset_due_to_mouse.x = dir.x * 10.0f;
  // camera_offset_due_to_mouse.y = dir.y * 10.0f;

  // Set position as the centered position of all targetss
  const auto& targets_view = r.view<const CameraFollow, const TransformComponent>();
  auto aggregate_pos = glm::vec2{ 0, 0 };
  int i = 0;
  for (const auto& [e, follow, t_c] : targets_view.each()) {
    aggregate_pos.x += t_c.position.x;
    aggregate_pos.y += t_c.position.y;
    i++;
  }
  if (i > 0) {
    aggregate_pos.x /= i;
    aggregate_pos.y /= i;
    // camera_transform.position.x = aggregate_pos.x;
    // camera_transform.position.y = aggregate_pos.y;

    const float speed = 20.0f; // higher number = faster to destination
    auto x = exp_decay(camera_transform.position.x, aggregate_pos.x, speed, dt);
    auto y = exp_decay(camera_transform.position.y, aggregate_pos.y, speed, dt);
    camera_transform.position.x = x;
    camera_transform.position.y = y;
  }

  // update lerp
  /*
  auto target_e = get_first<CameraLerpToTarget>(r);
  if (target_e != entt::null) {
    auto& lerp_c = r.get_or_emplace<LerpToMovingTarget>(camera_ent);
    lerp_c.a = get_position(r, camera_ent);
    lerp_c.b = get_position(r, target_e);
    lerp_c.speed = 5.0f;

    const auto d = lerp_c.a - lerp_c.b;
    const float d2 = d.x * d.x + d.y * d.y;
    const float threshold = 6;
    if (d2 <= threshold) {
      if (const auto* req = r.try_get<RemoveLerpWhenReachedTarget>(target_e)) {
        SDL_Log("Camera reached target... removing lerp");
        r.remove<CameraLerpToTarget>(target_e);
        r.remove<LerpToMovingTarget>(camera_ent);
        create_empty<CameraFreeMove>(r);
      }
    }

  } else
    remove_if_exists<LerpToMovingTarget>(r, camera_ent);
  */

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

  // Add camerashake
  const auto& screenshake_c = SINGLE_ScreenshakeComponent::instance;
  const auto screenshake_amount = screenshake_c.strength;

  const float screen_x = -ri.viewport_size_render_at.x * 0.5f;
  const float screen_y = -ri.viewport_size_render_at.y * 0.5f;

  // calculate view after updating postiion
  TransformComponent screen_offset = camera_transform;
  screen_offset.position.x = screen_x + screen_offset.position.x + screenshake_amount.x;
  screen_offset.position.y = screen_y + screen_offset.position.y + screenshake_amount.y;
  camera.view = calculate_ortho_view(screen_offset, dt);

  update_zoom(camera, dt);

  camera.projection_zoomed =
    calculate_ortho_projection(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y, camera.zoom_nonlinear);
};

void
update_camera_zoom_system(entt::registry& r, const float dt)
{
  auto& camera = get_first_component<OrthographicCamera>(r);
  camera.zoom_changed = false;
  auto& zoom = camera.zoom_linear;
  auto& zoom_nonlinear = camera.zoom_nonlinear;
  bool zoom_enabled = true;
  if (zoom_enabled) {
    // ImGui::Text("MouseWheel: %f", ImGui::GetIO().MouseWheel);
    if (ImGui::GetIO().MouseWheel > 0.0f) {
      zoom -= 0.1f;
      camera.zoom_changed = true;
    }
    if (ImGui::GetIO().MouseWheel < 0.0f) {
      zoom += 0.1f;
      camera.zoom_changed = true;
    }

    // hack: reset zoom.
    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
      zoom = 0.0f;
      zoom_nonlinear = 1.0f;
      camera.zoom_changed = true;
    }
  }
#if defined(_DEBUG)
  ImGui::Begin("DebugZoom");
  ImGui::SliderFloat("Zoom", &zoom, -1.0f, 1.0f);
  ImGui::End();
#endif
}

} // namespace game2d