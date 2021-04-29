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

// Game Thing: Randomly spawn things
// spawn_every_cooldown += delta_time_s;
// if (spawn_every_cooldown > spawn_every) {
//   float x = rand_det_s(rnd.rng, 0.0f, 1.0f);
//   float y = rand_det_s(rnd.rng, 0.0f, 1.0f);
//   glm::vec2 rand_pos(x * screen_width, y * screen_height);
//   printf("random pos: %f %f", rand_pos.x, rand_pos.y);
//   GameObject2D obj;
//   obj.pos = rand_pos;
//   objects.push_back(obj);
//   spawn_every_cooldown = 0.0f;
// }