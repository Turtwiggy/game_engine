// #pragma once

// game default states
// constexpr int GAME_OVER_WAVE = 10;
// constexpr int ENEMY_ATTACK_THRESHOLD = 4000;
// constexpr bool SPAWN_ENEMIES = true;
// constexpr int EXTRA_ENEMIES_TO_SPAWN_PER_WAVE = 5;
// constexpr int SECONDS_UNTIL_MAX_DIFFICULTY = 100;
// constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_START = 1.0f;
// constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_END = 0.2f;
// constexpr int ENEMY_BASE_HEALTH = 6;
// constexpr int ENEMY_BASE_DAMAGE = 2;
// constexpr int ENEMY_BASE_SPEED = 50;
// constexpr float screenshake_time = 0.1f;
// constexpr float vfx_flash_time = 0.2f;

// void
// player::scroll_to_swap_weapons(engine::Application& app, GameObject2D& player, std::vector<ShopItem>& inventory)
// {
//   float mousewheel = app.get_input().get_mousewheel_y();
//   float epsilon = 0.0001f;
//   if (mousewheel > epsilon || mousewheel < -epsilon) {
//     // int wheel_int = static_cast<int>(mousewheel);
//     // std::cout << "wheel int: " << wheel_int << "\n";
//     bool positive_direction = mousewheel > 0;

//     // cycle through weapons
//     int cur_item_index = player.equipped_item_index;
//     if (positive_direction)
//       cur_item_index = (cur_item_index + 1) % inventory.size();
//     else if (cur_item_index == 0)
//       cur_item_index = static_cast<int>(inventory.size() - 1);
//     else
//       cur_item_index = (cur_item_index - 1) % inventory.size();

//     player.equipped_item_index = cur_item_index;
//     // std::cout << "equipping item: " << cur_item_index << "mouse was pos: " << positive_direction << "\n";
//   }
// };
