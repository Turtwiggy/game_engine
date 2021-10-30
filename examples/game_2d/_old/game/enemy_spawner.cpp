// header
#include "enemy_spawner.hpp"

// other project headers
#include <glm/gtx/norm.hpp>

// game headers
#include "constants.hpp"

namespace game2d {

namespace enemy_spawner {

// difficulty: toughness
// difficulty: damage
// difficulty: speed
// spawn safe radius
const float game_safe_radius_around_player = 8000.0f;
// a wave
static std::map<int, float> wave_toughness{
  // clang-format off
  // 0-10 waves
  { 0, 0.5f },    
  { 1, 1.0f },    
  { 2, 1.15f },   
  { 3, 1.25f },   
  { 4, 1.35f },   
  { 5, 1.45f },   
  { 6, 1.55f },   
  { 7, 1.65f },   
  { 8, 1.8f  },   
  { 9, 2.0f },    
  // 10-20 waves
  { 10, 2.3f },
  { 11, 2.7f },
  { 12, 3.2f },
  { 13, 3.7f },
  { 14, 4.3f },
  { 15, 5.0f },
  { 16, 5.8f },
  { 17, 6.7f },
  { 18, 7.8f },
  { 19, 9.0f },
  // 20-27 waves  
  { 20, 10.4f },
  { 21, 12.0f },
  { 22, 13.8f },
  { 23, 15.9f },
  { 24, 18.3f },
  { 25, 21.0f },
  { 26, 24.3f },
  // clang-format on
};
static std::map<int, float> wave_damage{
  // clang-format off
  // 0-10 waves
  { 0, 0.5f },
  { 1, 1.0f },
  { 2, 1.1f },
  { 3, 1.2f },
  { 4, 1.3f },
  { 5, 1.4f },
  { 6, 1.5f },
  { 7, 1.6f },
  { 8, 1.7f },
  { 9, 1.8f },
  // 10-20 waves
  { 10, 2.1f },
  { 11, 2.5f },
  { 12, 2.9f },
  { 13, 3.4f },
  { 14, 4.0f },
  { 15, 4.6f },
  { 16, 5.3f },
  { 17, 6.1f },
  { 18, 7.1f },
  { 19, 8.2f },
  // 20-27 waves
  { 20, 9.5f },
  { 21, 11.0f },
  { 22, 12.7f },
  { 23, 14.7f },
  { 24, 17.0f },
  { 25, 19.6f },
  { 26, 22.6f },
  // clang-format on
};
static std::map<int, float> wave_speed{
  // clang-format off
  // 0-10 waves
  { 0, 0.8f },
  { 1, 1.0f },
  { 2, 1.2f },
  { 3, 1.35f },
  { 4, 1.45f },
  { 5, 1.55f },
  { 6, 1.65f },
  { 7, 1.75f },
  { 8, 1.85f },
  { 9, 2.0f },
  // 10-14 waves
  { 10, 2.0f },
  { 11, 2.3f },
  { 12, 2.7f },
  { 13, 3.2f },
  { 14, 3.5f }, // 3.5 is cap
  // clang-format on
};

} // namespace enemy_spawner

void
enemy_spawner::next_wave(int& enemies_to_spawn_this_wave, int& enemies_to_spawn_this_wave_left, int& wave)
{
  enemies_to_spawn_this_wave += EXTRA_ENEMIES_TO_SPAWN_PER_WAVE * wave;
  enemies_to_spawn_this_wave_left = enemies_to_spawn_this_wave;
  wave += 1;
  std::cout << "left: " << enemies_to_spawn_this_wave_left << std::endl;
}

// TODO fix this
void
spawn_enemy(MutableGameState& state, engine::RandomState& rnd, glm::vec2 world_pos)
{
  const int wave = state.wave;

  // spawn enemy
  GameObject2D enemy_copy = gameobject::create_enemy(rnd);
  // override defaults
  enemy_copy.pos = world_pos;

  // override stats based on wave

  if (wave < 28) {
    enemy_copy.damage_able_to_be_taken =
      static_cast<int>(ENEMY_BASE_HEALTH * enemy_spawner::wave_toughness[wave]); // toughness
    enemy_copy.damage_to_give_player = static_cast<int>(ENEMY_BASE_DAMAGE * enemy_spawner::wave_damage[wave]); // damage
  } else {
    std::cout << " This is the last (curated) wave...!" << std::endl;
    enemy_copy.damage_able_to_be_taken = static_cast<int>(ENEMY_BASE_HEALTH * 25.0f); // toughness
    enemy_copy.damage_to_give_player = static_cast<int>(ENEMY_BASE_DAMAGE * 25.0f);   // damage
  }

  if (wave < 15)
    enemy_copy.speed_current = ENEMY_BASE_SPEED * enemy_spawner::wave_speed[wave]; // speed
  else
    enemy_copy.speed_current = 3.5f;

  state.entities_enemies.push_back(enemy_copy);
}

void
enemy_spawner::update(MutableGameState& state,
                      engine::RandomState& rnd,
                      const glm::ivec2 screen_wh,
                      const float delta_time_s)
{
  state.game_enemy_seconds_between_spawning_left -= delta_time_s;
  if (state.game_enemy_seconds_between_spawning_left <= 0.0f) {
    state.game_enemy_seconds_between_spawning_left = state.game_enemy_seconds_between_spawning_current;

    // search params
    bool continue_search = true;
    int iterations_max = 3;
    int iteration = 0;
    // result
    float distance_squared = 0;
    glm::ivec2 found_pos = { 0.0, 0.0 };

    // generate random pos not too close to players
    do {

      // tried to generate X times
      if (iteration == iterations_max) {
        // ah, screw it, just spawn at 0, 0
        continue_search = false;
        std::cout << "(EnemySpawner) max iterations hit" << std::endl;
      }

      bool ok = true;
      glm::vec2 rnd_pos = glm::vec2(engine::rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_wh.x,
                                    engine::rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_wh.y);

      for (auto& player : state.entities_player) {

        distance_squared = glm::distance2(rnd_pos, glm::vec2(player.pos));
        ok = distance_squared > game_safe_radius_around_player;

        if (ok) {
          continue_search = false;
          found_pos = rnd_pos;
        }
        iteration += 1;
      }

    } while (continue_search);

    // std::cout << "enemy spawning " << distance_squared << " away from player" << std::endl;
    glm::vec2 world_pos = found_pos + state.camera.pos;

    if (SPAWN_ENEMIES && state.enemies_to_spawn_this_wave_left > 0) {
      spawn_enemy(state, rnd, world_pos);
      state.enemies_to_spawn_this_wave_left -= 1;
    }
  }

  // increase difficulty
  // 0.5 is starting cooldown
  // after 30 seconds, cooldown should be 0
  state.game_seconds_until_max_difficulty_spent += delta_time_s;
  float percent = glm::clamp(state.game_seconds_until_max_difficulty_spent / SECONDS_UNTIL_MAX_DIFFICULTY, 0.0f, 1.0f);
  state.game_enemy_seconds_between_spawning_current =
    glm::mix(SECONDS_BETWEEN_SPAWNING_ENEMIES_START, SECONDS_BETWEEN_SPAWNING_ENEMIES_END, percent);
};

} // namespace game2d