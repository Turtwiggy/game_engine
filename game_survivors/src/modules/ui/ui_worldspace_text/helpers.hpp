#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

#include <string>
#include <vector>

namespace game2d {

std::vector<std::string>
split_string_nearest_space(const std::string& str, const int& len);

entt::entity
create_popup(entt::registry& r, glm::vec2 pos, std::string text);

} // namespace game2d