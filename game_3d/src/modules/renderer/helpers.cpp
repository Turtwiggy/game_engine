#include "helpers.hpp"

namespace game2d {

glm::quat
vec3_to_quat(const glm::vec3& v)
{
  const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  const glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
  const glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
  const auto qx = glm::angleAxis(v.x, glm::vec3(1.0f, 0.0f, 0.0f));
  const auto qy = glm::angleAxis(v.y, glm::vec3(0.0f, 1.0f, 0.0f));
  const auto qz = glm::angleAxis(v.z, glm::vec3(0.0f, 0.0f, 1.0f));

  const float pitch = -v.x;
  const float yaw = -v.y;
  const float roll = -v.z;
  return glm::quat(glm::vec3(pitch, yaw, roll));
}

bool
check_if_viewport_resize(const SINGLE_RendererComponent& ri)
{
  const auto viewport_wh = ri.viewport_size_render_at;

  if (ri.viewport_size_current.x > 0.0f && ri.viewport_size_current.y > 0.0f &&
      (viewport_wh.x != ri.viewport_size_current.x || viewport_wh.y != ri.viewport_size_current.y)) {
    return true;
  }
  return false;
}

std::optional<TextureUnit>
search_for_texture_unit_by_texture_path(const SINGLE_RendererComponent& ri, const std::string& search)
{
  const auto result = std::find_if(ri.user_textures.begin(), ri.user_textures.end(), [&search](const Texture& tex) {
    return tex.path.find(search) != std::string::npos;
  });

  if (result != ri.user_textures.end())
    return result->tex_unit;

  return std::nullopt;
};

std::optional<TextureId>
search_for_texture_id_by_texture_path(const SINGLE_RendererComponent& ri, const std::string& search)
{
  const auto result = std::find_if(ri.user_textures.begin(), ri.user_textures.end(), [&search](const Texture& tex) {
    return tex.path.find(search) != std::string::npos;
  });

  if (result != ri.user_textures.end())
    return result->tex_id;

  return std::nullopt;
};

} // namespace game2d