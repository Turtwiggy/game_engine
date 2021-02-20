
#include "camera2d.hpp"

#include <glm/common.hpp>

namespace game2d {

float
linear_zoom_to_scale(float linear)
{
  return glm::pow(2.0f, linear);
}

float
logb(float base, float val)
{
  return glm::log(val) / glm::log(base);
}

float
scale_to_linear_zoom(float scale)
{
  return glm::log2(scale);
}

void
Camera2D::translate(glm::vec2 amount)
{
  pos += amount / calculate_scale();
}

void
Camera2D::zoom(float number_of_levels)
{
  zoom_level += number_of_levels;

  zoom_level = glm::clamp(zoom_level, -3.0f, 3.0f);
}

float
Camera2D::calculate_scale() const
{
  return linear_zoom_to_scale(zoom_level);
}

} // namespace game2d
