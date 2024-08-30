#include "orthographic.hpp"

#include "imgui.h"

namespace game2d {

glm::mat4
calculate_ortho_projection(int w, int h, const float dt)
{
  const float left = 0.0f;
  const float right = static_cast<float>(w);
  const float top = static_cast<float>(h);
  const float bottom = 0.0f;

  /*
  static float zoom = 1.0f;
  if (ImGui::GetIO().MouseWheel > 0.0f)
    zoom -= 10.0f * dt;
  if (ImGui::GetIO().MouseWheel < 0.0f)
    zoom += 10.0f * dt;
  ImGui::Text("zoom: %f", zoom);

  float aspect_ratio = 16.0f / 9.0f;
  const float l_zoom = left * zoom;
  const float r_zoom = right * zoom;
  const float b_zoom = bottom * zoom;
  const float t_zoom = top * zoom;
  return glm::ortho(l_zoom, r_zoom, b_zoom, t_zoom, -1.0f, 1.0f);
  */

  return glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
};

glm::mat4
calculate_ortho_view(const TransformComponent& transform, const float dt)
{
  const auto identity = glm::mat4(1.0f);
  const auto position = glm::vec3(transform.position.x, transform.position.y, transform.position.z);

  glm::mat4 view = glm::translate(identity, position);
  view *= glm::rotate(identity, transform.rotation_radians.z, glm::vec3(0, 0, 1));

  /*
  // temp: mess with zoom with scroll wheel
  static float zoom = 1.0f;
  if (ImGui::GetIO().MouseWheel > 0.0f)
    zoom -= 1.0f * dt;
  if (ImGui::GetIO().MouseWheel < 0.0f)
    zoom += 1.0f * dt;
  ImGui::Text("zoom: %f", zoom);
  // zoom = glm::clamp(zoom, -1.0f, 1.0f);
  // float amount = glm::pow(glm::sqrt(2.0f), zoom);
  // scale viewport from center
  const glm::vec2 size = { transform.scale.x, transform.scale.y };
  view = glm::translate(view, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
  view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
  view = glm::translate(view, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
  */

  return glm::inverse(view);
};

}