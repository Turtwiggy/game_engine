#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct CameraComponent
{
  glm::mat4 view;

  // perspective
  // float pitch = 0.0f;
  // float yaw = 0.0f;

  // orthographic
  float rotation = 0.0f;
};

// struct CameraTrackable
// {
//   bool placeholder = true;
// };

} // namespace game2d