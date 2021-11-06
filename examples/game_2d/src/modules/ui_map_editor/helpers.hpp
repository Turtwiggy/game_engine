#pragma once

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <iostream>

namespace game2d {
//

void
serialize_to_text(entt::registry& registry, std::string path);

void
deserialize_text_to_registry(entt::registry& registry, std::string path);

} // namespace game2d