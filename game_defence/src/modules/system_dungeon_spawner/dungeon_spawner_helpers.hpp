#pragma once

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

void
spawn_n_blackhole(entt::registry& r, std::vector<int>& idxs, int amount);

void
spawn_n_enemies(entt::registry& r, std::vector<int>& idxs, int amount);

void
spawn_n_players(entt::registry& r, std::vector<int>& idxs, int amount);

} // namespace game2d