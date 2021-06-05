#pragma once

// highscore
const std::string highscore_dir("runtime");
const std::string highscore_path(highscore_dir + "/highscores.txt");

// rotate to player
// glm::vec2 dir = player.pos - obj.pos;
// dir = glm::normalize(dir);
// obj.angle_radians = atan2(dir.y, dir.x);
// obj.angle_radians += fightingengine::PI / 2.0f;

// // TODO actually load highscore
// int
// load_highscore()
// {
//   std::ifstream highscores(highscore_path);

//   // int a;
//   // int highest = 0;
//   // while (highscores >> a) {
//   //   if (a > highest) {
//   //     highest = a;
//   //   }
//   // }

//   highscores.close();

//   return 10;
// }

// void
// save_highscore(const std::string& char_name, const int& score)
// {
//   // ImGui::Begin("Highscores");
//   // static char highscore_buf_temp[64] = "";
//   // ImGui::InputText("", highscore_buf_temp, 64);
//   // if (ImGui::Button("Save highscore")) {

//   std::cout << "saving highscore to: " << highscore_path << std::endl;
//   std::filesystem::create_directory(highscore_dir);
//   std::ofstream highscores(highscore_path, std::ofstream::out | std::ofstream::app);
//   highscores << "Character: " << char_name << " Score: " << score << std::endl;
//   highscores.close();
// }

// bool movement_spaceship_lrud = false;
// if (movement_spaceship_lrud) {
//   const float player_speed = 10.0f;
//   // Turn Ship
//   // if (app.get_input().get_key_held(SDL_SCANCODE_D))
//   //   player.angle += delta_time_s * angle_speed;
//   // if (app.get_input().get_key_held(SDL_SCANCODE_A))
//   //   player.angle -= delta_time_s * angle_speed;
//   if (app.get_input().get_key_held(SDL_SCANCODE_W))
//     player.velocity.y += player_speed;
//   if (app.get_input().get_key_held(SDL_SCANCODE_S))
//     player.velocity.y -= player_speed;
//   float turn_velocity_x = player.velocity.y; // same as y so turning doesnt feel weird
//   float turn_velocity_y = player.velocity.y;
//   float angle_speed = 200.0f;
//   if (app.get_input().get_key_held(SDL_SCANCODE_SPACE)) {
//     turn_velocity_x = 5.0f;
//     turn_velocity_y = 5.0f;
//     // increase turn speed
//     // angle_speed *= 5.0f;
//   }
// // update get vector based on angle
// // float x = glm::sin(player.angle_radians) * turn_velocity_x * extra_speed;
// // float y = -glm::cos(player.angle_radians) * turn_velocity_y * extra_speed;
// float x = glm::sin(mouse_angle_around_player) * turn_velocity_x * extra_speed;
// float y = -glm::cos(mouse_angle_around_player) * turn_velocity_y * extra_speed;
// player.pos.x += x * delta_time_s;
// player.pos.y += y * delta_time_s;
// }

//
// code graveyard
//

// bool
// aabb_collides(GameObject2D& one, GameObject2D& two)
// {
//   // collision x-axis?
//   bool collisionX = one.pos.x + one.size.x >= two.pos.x && two.pos.x + two.size.x >= one.pos.x;
//   // collision y-axis?
//   bool collisionY = one.pos.y + one.size.y >= two.pos.y && two.pos.y + two.size.y >= one.pos.y;
//   // collision only if on both axes
//   return collisionX && collisionY;
// }

// generate_collisions_bruteforce(std::vector<std::reference_wrapper<GameObject2D>>& objects,
//                                std::vector<std::pair<int, int>>& collisions)
// {
//   std::vector<std::reference_wrapper<GameObject2D>>::iterator it_1 = objects.begin();
//   while (it_1 != objects.end()) {
//     std::vector<std::reference_wrapper<GameObject2D>>::iterator it_2 = objects.begin();
//     while (it_2 != objects.end()) {
//       GameObject2D& obj_1 = *it_1;
//       GameObject2D& obj_2 = *it_2;
//       if (obj_1.id == obj_2.id) {
//         ++it_2;
//         continue;
//       }
//       bool collision_config = game_collision_matrix(obj_1.collision_layer, obj_2.collision_layer);
//       if (collision_config && aabb_collides(obj_1, obj_2)) {
//         std::pair<int, int> col;
//         col.first = obj_1.id;
//         col.second = obj_2.id;
//         collisions.push_back(col);
//       }
//       ++it_2;
//     }
//     ++it_1;
//   }
// };
