#pragma once

#include "modules/system_upgrade/upgrade_components.hpp"
#include <entt/entt.hpp>

namespace game2d {

SINGLE_Upgrades
load_upgrades(std::string path);

std::vector<std::string>
available_upgrade_names(entt::registry& r);

Upgrade
find_upgrade(entt::registry& r, const std::string& key);

std::string
generate_description(const Upgrade& u);

} // namespace game2d