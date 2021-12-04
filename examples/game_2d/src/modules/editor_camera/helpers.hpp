#pragma once

// components
#include "modules/editor_camera/components.hpp"

// other library headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace game2d {

inline glm::mat4
calculate_projection(int x, int y)
{
  return glm::ortho(0.0f, static_cast<float>(x), static_cast<float>(y), 0.0f, -1.0f, 1.0f);
};

inline void
update_view(CameraComponent& c)
{
  c.view = glm::lookAt(c.position, c.position + c.axis_forward, c.axis_up);
};

inline void
set_perspective(CameraComponent& c, float fov, float aspect, float near, float far)
{
  c.perspective = true;
  c.projection = glm::perspective(fov, aspect, near, far);
  c.fov = fov;
  c.aspect = aspect;
  c.near = near;
  c.far = far;
};

inline void
set_orthographic(CameraComponent& c, float left, float right, float top, float bottom, float near, float far)
{
  c.perspective = false;
  c.projection = glm::ortho(left, right, top, bottom, near, far);
  c.near = near;
  c.far = far;
};

inline glm::mat4
get_view_projection_matrix(const CameraComponent& c)
{
  return c.projection * c.view;
};

inline glm::mat4
get_inverse_projection_view_matrix(const CameraComponent& c)
{
  return glm::inverse(get_view_projection_matrix(c));
};

// // Compute the world direction vector based on the given X and Y coordinates in normalized-device space
// glm::vec3
// get_eye_ray(float x, float y) const
// {
//   glm::vec4 temp(x, y, 0.0f, 1.0f);

//   glm::mat4 inverse_projection_view = get_inverse_projection_view_matrix();

//   glm::vec4 ray = inverse_projection_view * temp;
//   ray /= ray.w;
//   ray.x -= this->position.x;
//   ray.y -= this->position.y;
//   ray.z -= this->position.z;

//   return glm::vec3(ray.x, ray.y, ray.z);
// }

// void
// process_mouse_input(float deltaX, float deltaY)
// {
//   float xmovement = deltaX * mouse_sensitivity;
//   float ymovement = deltaY * mouse_sensitivity;

//   target_yaw += xmovement;
//   target_pitch += (ymovement * -1);

//   // prevents calculating the length of the null vector
//   if (target_yaw == 0.0f)
//     target_yaw = 0.01f;
//   if (target_pitch == 0.0f)
//     target_pitch = 0.01f;

//   // it's not allowed to move the pitch above or below 90 degrees asctime the
//   // current world-up vector would break our LookAt calculation.
//   if (target_pitch > 89.0f)
//     target_pitch = 89.0f;
//   if (target_pitch < -89.0f)
//     target_pitch = -89.0f;
// }

// void
// process_scroll_input(float deltaX, float deltaY)
// {
//   movement_speed = glm::clamp(movement_speed + deltaY * 1.0f, 1.0f, 25.0f);
//   damping = glm::clamp(damping + deltaX * 0.5f, 1.0f, 25.0f);
// }

} // namespace game2d