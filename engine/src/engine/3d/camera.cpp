
// header
#include "engine/3d/camera.hpp"

// other library headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace fightingengine {

void
Camera::update_view()
{
  view = glm::lookAt(position, position + axis_forward, axis_up);
}

void
Camera::update(float delta_time)
{
  position = glm::lerp(position, target_position, glm::clamp(delta_time * damping, 0.0f, 1.0f));
  yaw = glm::lerp(yaw, target_yaw, glm::clamp(delta_time * damping * 2.0f, 0.0f, 1.0f));
  pitch = glm::lerp(pitch, target_pitch, glm::clamp(delta_time * damping * 2.0f, 0.0f, 1.0f));

  // calculate new cartesian basis vectors from yaw/pitch pair:
  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  axis_forward = glm::normalize(front);
  axis_right = glm::normalize(glm::cross(axis_forward, world_up));
  axis_up = glm::cross(axis_right, axis_forward);

  update_view();
}

void
Camera::process_mouse_input(float deltaX, float deltaY)
{
  float xmovement = deltaX * mouse_sensitivity;
  float ymovement = deltaY * mouse_sensitivity;

  target_yaw += xmovement;
  target_pitch += (ymovement * -1);

  // prevents calculating the length of the null vector
  if (target_yaw == 0.0f)
    target_yaw = 0.01f;
  if (target_pitch == 0.0f)
    target_pitch = 0.01f;

  // it's not allowed to move the pitch above or below 90 degrees asctime the
  // current world-up vector would break our LookAt calculation.
  if (target_pitch > 89.0f)
    target_pitch = 89.0f;
  if (target_pitch < -89.0f)
    target_pitch = -89.0f;
}

void
Camera::process_scroll_input(float deltaX, float deltaY)
{
  movement_speed = glm::clamp(movement_speed + deltaY * 1.0f, 1.0f, 25.0f);
  damping = glm::clamp(damping + deltaX * 0.5f, 1.0f, 25.0f);
}

void
Camera::set_perspective(float fov, float aspect, float near, float far)
{
  this->perspective = true;
  this->projection = glm::perspective(fov, aspect, near, far);
  this->fov = fov;
  this->aspect = aspect;
  this->near = near;
  this->far = far;
}

void
Camera::set_orthographic(float left, float right, float top, float bottom, float near, float far)
{
  this->perspective = false;
  this->projection = glm::ortho(left, right, top, bottom, near, far);
  this->near = near;
  this->far = far;
}

glm::mat4
Camera::get_view_projection_matrix() const
{
  return projection * view;
}

glm::mat4
Camera::get_inverse_projection_view_matrix() const
{
  return glm::inverse(get_view_projection_matrix());
}

// Compute the world direction vector based on the given X and Y coordinates in normalized-device space
glm::vec3
Camera::get_eye_ray(float x, float y) const
{
  glm::vec4 temp(x, y, 0.0f, 1.0f);

  glm::mat4 inverse_projection_view = get_inverse_projection_view_matrix();

  glm::vec4 ray = inverse_projection_view * temp;
  ray /= ray.w;
  ray.x -= this->position.x;
  ray.y -= this->position.y;
  ray.z -= this->position.z;

  return glm::vec3(ray.x, ray.y, ray.z);
}

void
camera_fly_around(Camera& camera, const float& dt, const Uint8* keyboard_state)
{
  // printf("direction: %s",
  // std::string(magic_enum::enum_name(direction)).c_str());
  float velocity = camera.movement_speed * dt;

  if (keyboard_state[SDL_Scancode::SDL_SCANCODE_W])
    camera.target_position += camera.axis_forward * velocity;
  else if (keyboard_state[SDL_Scancode::SDL_SCANCODE_S])
    camera.target_position -= camera.axis_forward * velocity;

  if (keyboard_state[SDL_Scancode::SDL_SCANCODE_A])
    camera.target_position -= camera.axis_right * velocity;
  else if (keyboard_state[SDL_Scancode::SDL_SCANCODE_D])
    camera.target_position += camera.axis_right * velocity;

  if (keyboard_state[SDL_Scancode::SDL_SCANCODE_SPACE])
    camera.target_position += camera.axis_up * velocity;
  else if (keyboard_state[SDL_Scancode::SDL_SCANCODE_LSHIFT])
    camera.target_position -= camera.axis_up * velocity;
}

void
camera_follow_position(Camera& camera, glm::vec3 position, glm::vec3 camera_follow_vec, const float& dt)
{
  float yaw_angle_around_player = 0.0f;
  float distance_from_player = 5.0f;

  camera.target_position = position + camera_follow_vec;
}

} // namespace fightingengine
