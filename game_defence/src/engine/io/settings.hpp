#pragma once

#include <entt/entt.hpp>

#include <optional>
#include <string>

namespace game2d {

const std::string SETTINGS_FILEPATH = "save.json";
const std::string PLAYERPREF_MUTE = "playerpref_mute";
const std::string SPACESHIP_COUNT = "spaceship_count";

void
save_string(const std::string& key, const std::string& value);

[[nodiscard]] std::optional<std::string>
get_string(const std::string& key);

[[nodiscard]] std::string
gesert_string(const std::string& key, const std::string& value);

} // namespace game2d