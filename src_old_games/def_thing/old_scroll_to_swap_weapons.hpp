// void
// player::scroll_to_swap_weapons(engine::Application& app, GameObject2D& player, std::vector<ShopItem>& inventory)
// {
//   float mousewheel = app.get_input().get_mousewheel_y();
//   float epsilon = 0.0001f;
//   if (mousewheel > epsilon || mousewheel < -epsilon) {
//     // int wheel_int = static_cast<int>(mousewheel);
//     // std::cout << "wheel int: " << wheel_int << std::endl;
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
//     // std::cout << "equipping item: " << cur_item_index << "mouse was pos: " << positive_direction << std::endl;
//   }
// };
