#pragma once

// other lib headers
#include <glm/glm.hpp>

// engine headers
#include "engine/application.hpp"

// game headers
#include "2d_game_object.hpp"
#include "2d_game_state.hpp"
#include "player_keys.hpp"
#include "weapons.hpp"

namespace game2d {

namespace player {

glm::ivec2
rotate_b_around_a(const GameObject2D& a, const GameObject2D& b, float radius, float angle);

void
scroll_to_swap_weapons(engine::Application& app, GameObject2D& player, std::vector<ShopItem>& inventory);

void
ability_boost(GameObject2D& player, const KeysAndState& keys, const float delta_time_s);

void
ability_slash(engine::Application& app,
              GameObject2D& player_obj,
              KeysAndState& keys,
              GameObject2D& weapon,
              float delta_time_s,
              MeleeWeaponStats& s,
              std::vector<Attack>& attacks);

void
player_attack(engine::Application& app,
              MutableGameState& gs,
              GameObject2D& player,
              std::vector<ShopItem>& player_inventory,
              KeysAndState& keys,
              const float delta_time_s,
              engine::RandomState& rnd);

} // namespace player

} // namespace game2d