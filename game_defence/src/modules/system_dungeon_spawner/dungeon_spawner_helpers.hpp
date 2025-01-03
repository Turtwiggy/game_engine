#pragma once

#include "modules/ui_units/ui_units_components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

std::vector<entt::entity>
spawn_n_blackhole(entt::registry& r, std::vector<int>& idxs, int amount);

std::vector<entt::entity>
spawn_n_enemies(entt::registry& r, std::vector<int>& idxs, int amount);

std::vector<entt::entity>
spawn_n_players(entt::registry& r, std::vector<int>& idxs, const std::vector<UnitType>& units);

} // namespace game2d