#pragma once

#include "modules/ui_units/ui_units_components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

void
spawn_n_blackhole(entt::registry& r, std::vector<int>& idxs, int amount);

void
spawn_n_enemies(entt::registry& r, std::vector<int>& idxs, int amount);

void
spawn_n_players(entt::registry& r, std::vector<int>& idxs);

} // namespace game2d