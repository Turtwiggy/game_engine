#pragma once

#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

SINGLE_PersistentUpgrades
load_upgrades(std::string path);

std::vector<std::string>
available_upgrade_names(entt::registry& r);

Upgrade
find_upgrade(entt::registry& r, const std::string& key);

//
//
//

void
load_persistent_upgrades_and_apply_to_player(entt::registry& r);

// std::string
// generate_description(const Upgrade& u);

} // namespace game2d