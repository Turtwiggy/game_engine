
// your header
#include "2d_game_logic.hpp"

// c++ lib headers
#include <iostream>

// other lib headers
#include <glm/gtx/vector_angle.hpp> // for distance2

namespace game2d {

void
bullet::update(GameObject2D& obj, float delta_time_s)
{
  gameobject::update_position(obj, delta_time_s);

  // look in velocity direction
  float angle = atan2(obj.velocity.y, obj.velocity.x);
  angle += fightingengine::HALF_PI + sprite::spritemap::get_sprite_rotation_offset(obj.sprite);
  obj.angle_radians = angle;
};

void
camera::update(GameObject2D& camera, const KeysAndState& keys, fightingengine::Application& app, float delta_time_s)
{
  // go.pos = glm::vec2(other.pos.x - screen_width / 2.0f, other.pos.y - screen_height / 2.0f);
  if (app.get_input().get_key_held(keys.key_camera_left))
    camera.pos.x -= delta_time_s * camera.speed_current;
  if (app.get_input().get_key_held(keys.key_camera_right))
    camera.pos.x += delta_time_s * camera.speed_current;
  if (app.get_input().get_key_held(keys.key_camera_up))
    camera.pos.y -= delta_time_s * camera.speed_current;
  if (app.get_input().get_key_held(keys.key_camera_down))
    camera.pos.y += delta_time_s * camera.speed_current;
};

namespace collisions {

void
resolve_enemy_player_collision(GameObject2D& enemy, GameObject2D& player)
{
  std::cout << "enemy-player collision!" << std::endl;

  //
  // enemy
  //

  enemy.flag_for_delete = true;

  //
  // player
  //

  player.hits_taken += 1;
}

void
resolve_enemy_bullet_collision(GameObject2D& enemy, GameObject2D& bullet)
{
  std::cout << "enemy-bullet collision!" << std::endl;

  //
  // enemy
  //

  enemy.hits_taken += 1;

  //
  // bullet
  //

  bullet.flag_for_delete = true;
}

void
resolve(uint32_t id0, uint32_t id1, const std::vector<std::reference_wrapper<GameObject2D>>& ents)
{
  // Find the objs in the read-only list
  auto& obj_0_it = std::find_if(ents.begin(), ents.end(), [&id0](const auto& obj) { return obj.get().id == id0; });
  auto& obj_1_it = std::find_if(ents.begin(), ents.end(), [&id1](const auto& obj) { return obj.get().id == id1; });

  if (obj_0_it == ents.end() || obj_1_it == ents.end()) {
    std::cerr << "Collision entity not in entity list" << std::endl;
  }

  auto& coll_layer_0 = obj_0_it->get().collision_layer;
  auto& coll_layer_1 = obj_1_it->get().collision_layer;

  //
  // apply to collision interactions e.g. xy or yx
  //

  // Enemy and player collided
  if ((coll_layer_0 == CollisionLayer::Player && coll_layer_1 == CollisionLayer::Enemy) ||
      (coll_layer_1 == CollisionLayer::Player && coll_layer_0 == CollisionLayer::Enemy)) {

    // work out which is which
    if (obj_0_it->get().collision_layer == CollisionLayer::Enemy) {
      resolve_enemy_player_collision(obj_0_it->get(), obj_1_it->get());
    } else {
      resolve_enemy_player_collision(obj_1_it->get(), obj_0_it->get());
    }
  }

  // Enemy and bullet
  if ((coll_layer_0 == CollisionLayer::Enemy && coll_layer_1 == CollisionLayer::Bullet) ||
      (coll_layer_1 == CollisionLayer::Enemy && coll_layer_0 == CollisionLayer::Bullet)) {

    // work out which is which
    if (obj_0_it->get().collision_layer == CollisionLayer::Enemy) {
      resolve_enemy_bullet_collision(obj_0_it->get(), obj_1_it->get());
    } else {
      resolve_enemy_bullet_collision(obj_1_it->get(), obj_0_it->get());
    }
  }
};

} // namespace collisions

void
enemy_ai::move_along_vector(GameObject2D& obj, glm::vec2 dir, float delta_time_s)
{
  dir = glm::normalize(dir);
  obj.pos += (dir * obj.speed_current * delta_time_s);
};

void
enemy_ai::enemy_directly_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s)
{
  glm::vec2 ab = player.pos - obj.pos;
  move_along_vector(obj, ab, delta_time_s);
};

void
enemy_ai::enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s)
{
  // calculate a vector ab
  glm::vec2 ab = player.pos - obj.pos;
  // calculate the point halfway between ab
  glm::vec2 half_point = obj.pos + (ab / 2.0f);
  // calculate the vector at a right angle
  glm::vec2 normal = glm::vec2(-ab.y, ab.x);

  // expensive(?) distance calc
  float distance = glm::distance(obj.pos, player.pos);
  float half_distance = distance / 2.0f;

  // offset the midpoint via normal
  float amplitude = half_distance * sin(obj.approach_theta_degrees);
  half_point += (glm::normalize(normal) * amplitude);

  // Now create a bezier curve! use the halfpoint as the control point
  // float t = 0.5f;
  // glm::vec2 p = quadratic_curve(obj.pos, half_point, player.pos, t);

  glm::vec2 dir = glm::normalize(half_point - obj.pos);
  // std::cout << "dir x:" << dir.x << " y:" << dir.y << std::endl;

  move_along_vector(obj, dir, delta_time_s);
};

namespace enemy_spawner {

const bool game_spawn_enemies = true;
const float game_wall_seconds_between_spawning_start = 0.5f;
float game_wall_seconds_between_spawning_current = game_wall_seconds_between_spawning_start;
float game_wall_seconds_between_spawning_left = 0.0f;
const float game_wall_seconds_between_spawning_end = 0.2f;
const float game_seconds_until_max_difficulty = 100.0f;
float game_seconds_until_max_difficulty_spent = 0.0f;

void
update(std::vector<GameObject2D>& enemies,
       GameObject2D& camera,
       std::vector<GameObject2D>& players,
       fightingengine::RandomState& rnd,
       const glm::ivec2 screen_wh,
       const float safe_radius_around_player,
       const int tex_unit,
       const glm::vec4 col,
       const sprite::type sprite,
       const float delta_time_s)
{
  game_wall_seconds_between_spawning_left -= delta_time_s;
  if (game_wall_seconds_between_spawning_left <= 0.0f) {
    game_wall_seconds_between_spawning_left = game_wall_seconds_between_spawning_current;

    // search params
    bool continue_search = true;
    int iterations_max = 3;
    int iteration = 0;
    // result
    float distance_squared = 0;
    glm::vec2 found_pos = { 0.0f, 0.0f };

    // generate random pos not too close to players
    do {

      // tried to generate X times
      if (iteration == iterations_max) {
        // ah, screw it, just spawn at 0, 0
        continue_search = false;
        std::cout << "(EnemySpawner) max iterations hit" << std::endl;
      }

      bool ok = true;
      glm::vec2 rnd_pos = glm::vec2(fightingengine::rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_wh.x,
                                    fightingengine::rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_wh.y);

      for (auto& player : players) {

        distance_squared = glm::distance2(rnd_pos, player.pos);
        ok = distance_squared > safe_radius_around_player;

        if (ok) {
          continue_search = false;
          found_pos = rnd_pos;
        }
        iteration += 1;
      }

    } while (continue_search);

    // std::cout << "enemy spawning " << distance_squared << " away from player" << std::endl;
    glm::vec2 world_pos = found_pos + camera.pos;

    if (game_spawn_enemies) {
      // spawn enemy
      GameObject2D wall_copy = gameobject::create_enemy(sprite, tex_unit, col, rnd);
      // override defaults
      wall_copy.pos = world_pos;
      enemies.push_back(wall_copy);
    }
  }

  // increase difficulty
  // 0.5 is starting cooldown
  // after 30 seconds, cooldown should be 0
  game_seconds_until_max_difficulty_spent += delta_time_s;
  float percent =
    glm::clamp(game_seconds_until_max_difficulty_spent / game_seconds_until_max_difficulty_spent, 0.0f, 1.0f);
  game_wall_seconds_between_spawning_current =
    glm::mix(game_wall_seconds_between_spawning_start, game_wall_seconds_between_spawning_end, percent);
};

} // namespace enemyspawner

namespace player {

bool game_player_shoot = false; // affects all players

void
update_input(GameObject2D& obj, KeysAndState& keys, fightingengine::Application& app, GameObject2D& camera)
{
  keys.l_analogue_x = 0.0f;
  keys.l_analogue_y = 0.0f;
  keys.r_analogue_x = 0.0f;
  keys.r_analogue_y = 0.0f;
  keys.shoot_pressed = false;
  keys.boost_pressed = false;
  keys.pause_pressed = false;

  // Keymaps: keyboard
  if (keys.use_keyboard) {
    if (app.get_input().get_key_held(keys.w)) {
      keys.l_analogue_y = -1.0f;
    } else if (app.get_input().get_key_held(keys.s)) {
      keys.l_analogue_y = 1.0f;
    } else {
      keys.l_analogue_y = 0.0f;
    }

    if (app.get_input().get_key_held(keys.a)) {
      keys.l_analogue_x = -1.0f;
    } else if (app.get_input().get_key_held(keys.d)) {
      keys.l_analogue_x = 1.0f;
    } else {
      keys.l_analogue_x = 0.0f;
    }

    keys.shoot_pressed = app.get_input().get_mouse_lmb_held();
    keys.boost_pressed = app.get_input().get_key_held(keys.key_boost);
    keys.pause_pressed = app.get_input().get_key_down(keys.key_pause);

    glm::vec2 player_world_space_pos = gameobject_in_worldspace(camera, obj);
    float mouse_angle_around_player = atan2(app.get_input().get_mouse_pos().y - player_world_space_pos.y,
                                            app.get_input().get_mouse_pos().x - player_world_space_pos.x);

    mouse_angle_around_player += fightingengine::HALF_PI;
    keys.angle_around_player = mouse_angle_around_player;

    float x_axis = glm::sin(mouse_angle_around_player);
    float y_axis = -glm::cos(mouse_angle_around_player);
    keys.r_analogue_x = x_axis;
    keys.r_analogue_y = y_axis;
  }

  // }
  // // Keymaps: Controller
  // else {
  //   l_analogue_x = app.get_input().get_axis_dir(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX);
  //   l_analogue_y = app.get_input().get_axis_dir(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY);
  //   r_analogue_x = app.get_input().get_axis_dir(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX);
  //   r_analogue_y = app.get_input().get_axis_dir(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY);
  //   shoot_pressed =
  //     app.get_input().get_axis_held(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
  //   boost_pressed =
  //     app.get_input().get_axis_held(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT);
  //   pause_pressed =
  //     app.get_input().get_button_held(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START);

  //   look_angle = atan2(r_analogue_y, r_analogue_x);
  //   look_angle += HALF_PI;
  // }
};

void
ability_boost(GameObject2D& player, const KeysAndState& keys, const float delta_time_s)
{
  if (keys.boost_pressed) {
    // Boost when shift pressed
    player.shift_boost_time_left -= delta_time_s;
    player.shift_boost_time_left = player.shift_boost_time_left < 0.0f ? 0.0f : player.shift_boost_time_left;
  } else {
    // Recharge when shift released
    player.shift_boost_time_left += delta_time_s;
    // Cap limit
    player.shift_boost_time_left =
      player.shift_boost_time_left > player.shift_boost_time ? player.shift_boost_time : player.shift_boost_time_left;
  }

  if (keys.boost_pressed && player.shift_boost_time_left > 0.0f) {
    player.velocity *= player.velocity_boost_modifier;
  }
}

void
ability_shoot(GameObject2D& player,
              const KeysAndState& keys,
              std::vector<GameObject2D>& bullets,
              const int tex_unit,
              const glm::vec4 col,
              const sprite::type sprite,
              const float delta_time_s)
{
  // Ability: Shoot
  // if (keys.shoot_pressed)
  //   obj.bullets_to_fire_after_releasing_mouse_left = obj.bullets_to_fire_after_releasing_mouse;

  if (game_player_shoot) {
    if (player.bullet_seconds_between_spawning_left > 0.0f)
      player.bullet_seconds_between_spawning_left -= delta_time_s;

    if (player.bullet_seconds_between_spawning_left <= 0.0f) {
      player.bullet_seconds_between_spawning_left = player.bullet_seconds_between_spawning;
      // obj.bullets_to_fire_after_releasing_mouse_left -= 1;
      // obj.bullets_to_fire_after_releasing_mouse_left =
      //   obj.bullets_to_fire_after_releasing_mouse_left < 0 ? 0 : obj.bullets_to_fire_after_releasing_mouse_left;

      // spawn bullet

      GameObject2D bullet_copy = gameobject::create_bullet(sprite, tex_unit, col);
      // override defaults
      // fix offset issue so bullet spawns in middle of player
      glm::vec2 bullet_pos = player.pos;
      bullet_pos.x += player.size.x / 2.0f - bullet_copy.size.x / 2.0f;
      bullet_pos.y += player.size.y / 2.0f - bullet_copy.size.y / 2.0f;
      bullet_copy.pos = bullet_pos;
      // convert right analogue input to velocity
      bullet_copy.velocity.x = keys.r_analogue_x * bullet_copy.speed_current;
      bullet_copy.velocity.y = keys.r_analogue_y * bullet_copy.speed_current;

      bullets.push_back(bullet_copy);
    }
  }
}

void
update_logic(GameObject2D& player,
             const KeysAndState& keys,
             std::vector<GameObject2D>& bullets,
             const int tex_unit,
             const glm::vec4 col,
             const sprite::type sprite,
             GameObject2D& weapon,
             const float delta_time_s)
{
  // process input
  player.velocity.x = keys.l_analogue_x;
  player.velocity.y = keys.l_analogue_y;
  player.velocity *= player.speed_current;

  ability_boost(player, keys, delta_time_s);
  ability_shoot(player, keys, bullets, tex_unit, col, sprite, delta_time_s);

  gameobject::update_position(player, delta_time_s);

  // ability: weapon
  glm::vec2 pos = player.pos;
  pos.x += player.size.x / 2.0f - weapon.size.x / 2.0f;
  pos.y += player.size.y / 2.0f - weapon.size.y / 2.0f;
  weapon.pos = pos;
  weapon.angle_radians = keys.angle_around_player + sprite::spritemap::get_sprite_rotation_offset(weapon.sprite);

  // calculate a vector ab
  // glm::vec2 ab = player.pos - glm::vec2(keys.angle_around_player);

  // offset weapon in direction of look angle

  // fix offset issue so bullet spawns in middle of player
  // glm::vec2 bullet_pos = player.pos;
  // bullet_pos.x += player.size.x / 2.0f - bullet_copy.size.x / 2.0f;
  // bullet_pos.y += player.size.y / 2.0f - bullet_copy.size.y / 2.0f;
  // bullet_copy.pos = bullet_pos;

  // update colour
  // float t = (player.hits_taken) / static_cast<float>(player.hits_able_to_be_taken);
  // t = glm::clamp(t, 0.0f, 1.0f); // clamp it
  // glm::vec4 col = glm::mix(player_colour, player_dead_colour, t);
  // float min_alpha = 0.7f;
  // col.a = glm::clamp(1.0f - t, min_alpha, 1.0f);
  // player.colour = col;
};

} // namespace player

} // namespace game2d
