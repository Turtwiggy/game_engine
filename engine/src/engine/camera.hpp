#pragma once

// c++ standard library headers
#include <vector>

// other library headers
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fightingengine {

class Camera
{
public:
  glm::vec3 target_position = glm::vec3(0.0f, 0.0f, 0.0f);
  float target_yaw = 0.0f;
  float target_pitch = 0.0f;

  float damping = 5.0f;
  float movement_speed = 2.0f;
  float mouse_speed = 2.5f;
  float mouse_sensitivity = 0.1f;

  glm::vec3 axis_forward = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 axis_up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 axis_right = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

private:
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  float yaw = 0.0f;
  float pitch = 0.0f;

  glm::mat4 projection;
  glm::mat4 view;
  bool perspective;
  float fov;
  float aspect;
  float near;
  float far;

public:
  Camera() = default;

  void update(float delta_time);
  void update_view();

  void process_mouse_input(float deltaX, float deltaY);
  void process_scroll_input(float deltaX, float deltaY);

  void set_perspective(float fov, float aspect, float near, float far);
  void set_orthographic(float left, float right, float top, float bottom, float near, float far);

  glm::mat4 get_view_projection_matrix() const;
  glm::mat4 get_inverse_projection_view_matrix() const;

  glm::vec3 get_eye_ray(float x, float y) const;

  glm::vec3 get_position() { return position; }
  float get_yaw() { return yaw; }
  float get_pitch() { return pitch; }
};

// Camera dictates it's own position and lets the user fly around
void
camera_fly_around(Camera& camera, const float& dt, const Uint8* keyboard_state);

// Camera is given a position to follow
void
camera_follow_position(Camera& camera, glm::vec3 position, const float& dt);

} // namespace fightingengine
