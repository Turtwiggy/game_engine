
// // game headers
// #include "constants.hpp"

// namespace enemy_spawner {

// // difficulty: toughness
// // difficulty: damage
// // difficulty: speed
// // spawn safe radius
// const float game_safe_radius_around_player = 8000.0f;
// // a wave
// static std::map<int, float> wave_toughness{};
// static std::map<int, float> wave_damage{};
// static std::map<int, float> wave_speed{};

// } // namespace enemy_spawner

// void
// enemy_spawner::next_wave(int& enemies_to_spawn_this_wave, int& enemies_to_spawn_this_wave_left, int& wave)
// {
//   enemies_to_spawn_this_wave += EXTRA_ENEMIES_TO_SPAWN_PER_WAVE * wave;
//   enemies_to_spawn_this_wave_left = enemies_to_spawn_this_wave;
//   wave += 1;
//   std::cout << "left: " << enemies_to_spawn_this_wave_left << std::endl;
// }

// void
// spawn_enemy(MutableGameState& state, engine::RandomState& rnd, glm::vec2 world_pos)
// {
//   const int wave = state.wave;

//   // spawn enemy
//   GameObject2D enemy_copy = gameobject::create_enemy(rnd);
//   // override defaults
//   enemy_copy.pos = world_pos;

//   // override stats based on wave

//   if (wave < 28) {
//     enemy_copy.damage_able_to_be_taken =
//       static_cast<int>(ENEMY_BASE_HEALTH * enemy_spawner::wave_toughness[wave]); // toughness
//     enemy_copy.damage_to_give_player = static_cast<int>(ENEMY_BASE_DAMAGE * enemy_spawner::wave_damage[wave]); //
//     damage
//   } else {
//     std::cout << " This is the last (curated) wave...!" << std::endl;
//     enemy_copy.damage_able_to_be_taken = static_cast<int>(ENEMY_BASE_HEALTH * 25.0f); // toughness
//     enemy_copy.damage_to_give_player = static_cast<int>(ENEMY_BASE_DAMAGE * 25.0f);   // damage
//   }

//   if (wave < 15)
//     enemy_copy.speed_current = ENEMY_BASE_SPEED * enemy_spawner::wave_speed[wave]; // speed
//   else
//     enemy_copy.speed_current = 3.5f;

//   state.entities_enemies.push_back(enemy_copy);
// }

// void
// enemy_spawner::update(MutableGameState& state,
//                       engine::RandomState& rnd,
//                       const glm::ivec2 screen_wh,
//                       const float delta_time_s)
// {
//   state.game_enemy_seconds_between_spawning_left -= delta_time_s;
//   if (state.game_enemy_seconds_between_spawning_left <= 0.0f) {
//     state.game_enemy_seconds_between_spawning_left = state.game_enemy_seconds_between_spawning_current;

//     // search params
//     bool continue_search = true;
//     int iterations_max = 3;
//     int iteration = 0;
//     // result
//     float distance_squared = 0;
//     glm::ivec2 found_pos = { 0.0, 0.0 };

//     // generate random pos not too close to players
//     do {

//       // tried to generate X times
//       if (iteration == iterations_max) {
//         // ah, screw it, just spawn at 0, 0
//         continue_search = false;
//         std::cout << "(EnemySpawner) max iterations hit" << std::endl;
//       }

//       bool ok = true;
//       glm::vec2 rnd_pos = glm::vec2(engine::rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_wh.x,
//                                     engine::rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_wh.y);

//       for (auto& player : state.entities_player) {

//         distance_squared = glm::distance2(rnd_pos, glm::vec2(player.pos));
//         ok = distance_squared > game_safe_radius_around_player;

//         if (ok) {
//           continue_search = false;
//           found_pos = rnd_pos;
//         }
//         iteration += 1;
//       }

//     } while (continue_search);

//     // std::cout << "enemy spawning " << distance_squared << " away from player" << std::endl;
//     glm::vec2 world_pos = found_pos + state.camera.pos;

//     if (SPAWN_ENEMIES && state.enemies_to_spawn_this_wave_left > 0) {
//       spawn_enemy(state, rnd, world_pos);
//       state.enemies_to_spawn_this_wave_left -= 1;
//     }
//   }

//   // increase difficulty
//   // 0.5 is starting cooldown
//   // after 30 seconds, cooldown should be 0
//   state.game_seconds_until_max_difficulty_spent += delta_time_s;
//   float percent = glm::clamp(state.game_seconds_until_max_difficulty_spent / SECONDS_UNTIL_MAX_DIFFICULTY,
//   0.0f, 1.0f); state.game_enemy_seconds_between_spawning_current =
//     glm::mix(SECONDS_BETWEEN_SPAWNING_ENEMIES_START, SECONDS_BETWEEN_SPAWNING_ENEMIES_END, percent);
// };