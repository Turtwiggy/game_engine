#pragma once

#include "components.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <optional>

namespace game2d {

glm::quat
vec3_to_quat(const glm::vec3& v);

bool
check_if_viewport_resize(const SINGLE_RendererComponent& ri);

std::optional<TextureUnit>
search_for_texture_unit_by_texture_path(const SINGLE_RendererComponent& ri, const std::string& search);

std::optional<game2d::TextureId>
search_for_texture_id_by_texture_path(const game2d::SINGLE_RendererComponent& ri, const std::string& search);

} // namespace game2d