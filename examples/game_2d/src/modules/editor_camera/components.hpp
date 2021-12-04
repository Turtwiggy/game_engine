#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct CameraComponent
{
  float fov = 45.0f, aspect = 16 / 9.0f, near = 0.1f, far = 1000.0f;
  float yaw = 0.0f, pitch = 0.0f;
  float target_yaw = 0.0f, target_pitch = 0.0f;
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 target_position = glm::vec3(0.0f, 0.0f, 0.0f);
  float damping = 5.0f;
  float movement_speed = 2.0f;
  float mouse_speed = 2.5f;
  float mouse_sensitivity = 0.1f;
  glm::vec3 axis_forward = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 axis_up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 axis_right = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::mat4 view;
  glm::mat4 projection;
  bool perspective = false;
};

struct EditorCameraComponent
{
  bool placeholder = true;
};

} // namespace game2d