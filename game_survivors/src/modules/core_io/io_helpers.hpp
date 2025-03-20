#pragma once

#include "io_components.hpp"

#include <entt/fwd.hpp>
#include <optional>

namespace game2d {

SINGLE_OnDiskData
savefile_load_disk(entt::registry& r);
void
savefile_save_disk(entt::registry& r);

std::optional<nlohmann::json>
savefile_get_key(entt::registry& r, const std::string& str);
void
savefile_put_key(entt::registry& r, const std::string& key, const nlohmann::json& val);

} // namespace game2d