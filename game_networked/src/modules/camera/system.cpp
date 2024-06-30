// header
#include "system.hpp"

// components/systems
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/camera/perspective.hpp"

#include <glm/glm.hpp>

namespace game2d {

void
update_camera_system(engine::GameWindow& window, entt::registry& r, const float dt)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

  const auto camera_e = get_first<PerspectiveCamera>(r);
  auto& camera = r.get<PerspectiveCamera>(camera_e);
  auto& camera_transform = r.get<TransformComponent>(camera_e);

  const float velocity = camera.speed * dt;

  const auto mouse_input = window.get_relative_mouse_position();

  if (get_key_down(input, SDL_SCANCODE_M))
    window.toggle_mouse_capture();

  if (get_key_held(input, SDL_SCANCODE_W))
    camera_transform.position += get_forward_dir(camera) * velocity;
  if (get_key_held(input, SDL_SCANCODE_S))
    camera_transform.position -= get_forward_dir(camera) * velocity;
  if (get_key_held(input, SDL_SCANCODE_A))
    camera_transform.position -= get_right_dir(camera) * velocity;
  if (get_key_held(input, SDL_SCANCODE_D))
    camera_transform.position += get_right_dir(camera) * velocity;
  if (get_key_held(input, SDL_SCANCODE_SPACE))
    camera_transform.position += get_up_dir(camera) * velocity;
  if (get_key_held(input, SDL_SCANCODE_LSHIFT))
    camera_transform.position -= get_up_dir(camera) * velocity;

  if (window.get_mouse_captured()) {
    const float mouse_sens = 0.01f;
    const float mouse_input_x = mouse_input.x * mouse_sens;
    camera.yaw += mouse_input_x;

    const float mouse_input_y = mouse_input.y * mouse_sens;
    camera.pitch += mouse_input_y;

    if (camera.pitch > 89.0f)
      camera.pitch = 89.0f;
    if (camera.pitch < -89.0f)
      camera.pitch = -89.0f;
  }

  camera.view = calculate_perspective_view(camera_transform, camera);

  // if (get_mouse_mmb_held()) { // pan
  // if (get_mouse_lmb_held()) { // rotate
  // if (get_mouse_rmb_held()) { // zoom
};

} // namespace