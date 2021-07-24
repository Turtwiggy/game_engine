
// your header
#include "2d_game_logic.hpp"

// c++ lib headers
#include <iostream>

// other lib headers
#include <glm/gtx/vector_angle.hpp> // for distance2

// engine headers
#include "engine/maths_core.hpp"

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
  float amplitude = half_distance * sin(glm::radians(obj.approach_theta_degrees));
  half_point += (glm::normalize(normal) * amplitude);

  glm::vec2 dir = glm::normalize(half_point - obj.pos);

  move_along_vector(obj, dir, delta_time_s);
};

namespace enemy_spawner {

const bool game_spawn_enemies = true;
const float game_wall_seconds_between_spawning_start = 1.0f;
float game_wall_seconds_between_spawning_current = game_wall_seconds_between_spawning_start;
float game_wall_seconds_between_spawning_left = 0.0f;
const float game_wall_seconds_between_spawning_end = 0.2f;
const float game_seconds_until_max_difficulty = 100.0f;
float game_seconds_until_max_difficulty_spent = 0.0f;

void
next_wave(int& enemies_to_spawn_this_wave, int& enemies_to_spawn_this_wave_left)
{
  enemies_to_spawn_this_wave += 5;
  enemies_to_spawn_this_wave_left = enemies_to_spawn_this_wave;
  std::cout << "left: " << enemies_to_spawn_this_wave_left << std::endl;
}

void
update(std::vector<GameObject2D>& enemies,
       std::vector<GameObject2D>& players,
       int& enemies_to_spawn,
       const GameObject2D& camera,
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

    if (game_spawn_enemies && enemies_to_spawn > 0) {
      // spawn enemy
      GameObject2D wall_copy = gameobject::create_enemy(sprite, tex_unit, col, rnd);
      // override defaults
      wall_copy.pos = world_pos;
      enemies.push_back(wall_copy);

      enemies_to_spawn -= 1;
    }
  }

  // increase difficulty
  // 0.5 is starting cooldown
  // after 30 seconds, cooldown should be 0
  game_seconds_until_max_difficulty_spent += delta_time_s;
  float percent = glm::clamp(game_seconds_until_max_difficulty_spent / game_seconds_until_max_difficulty, 0.0f, 1.0f);
  game_wall_seconds_between_spawning_current =
    glm::mix(game_wall_seconds_between_spawning_start, game_wall_seconds_between_spawning_end, percent);
};

} // namespace enemyspawner

namespace player {

void
update_input(GameObject2D& obj, KeysAndState& keys, fightingengine::Application& app, GameObject2D& camera)
{
  keys.l_analogue_x = 0.0f;
  keys.l_analogue_y = 0.0f;
  keys.r_analogue_x = 0.0f;
  keys.r_analogue_y = 0.0f;

  keys.shoot_down = false;
  keys.shoot_held = false;
  keys.boost_down = false;
  keys.boost_held = false;
  keys.pause_down = false;
  keys.pause_held = false;

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

    keys.shoot_down = app.get_input().get_mouse_lmb_down();
    keys.shoot_held = app.get_input().get_mouse_lmb_held();
    keys.boost_down = app.get_input().get_key_down(keys.key_boost);
    keys.boost_held = app.get_input().get_key_held(keys.key_boost);
    keys.pause_down = app.get_input().get_key_down(keys.key_pause);
    keys.pause_held = app.get_input().get_key_held(keys.key_pause);

    glm::vec2 player_world_space_pos = gameobject_in_worldspace(camera, obj);
    float mouse_angle_around_player = atan2(app.get_input().get_mouse_pos().y - player_world_space_pos.y,
                                            app.get_input().get_mouse_pos().x - player_world_space_pos.x);

    mouse_angle_around_player += fightingengine::HALF_PI;
    keys.angle_around_player = mouse_angle_around_player;

    float x_axis = glm::sin(mouse_angle_around_player);
    float y_axis = -glm::cos(mouse_angle_around_player);
    keys.r_analogue_x = x_axis;
    keys.r_analogue_y = y_axis;
  };

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
  if (keys.boost_held) {
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

  if (keys.boost_held && player.shift_boost_time_left > 0.0f) {
    player.velocity *= player.velocity_boost_modifier;
  }
};

void
ability_shoot(fightingengine::Application& app,
              GameObject2D& fire_from_this_entity,
              const KeysAndState& keys,
              std::vector<GameObject2D>& bullets,
              const int tex_unit,
              const glm::vec4 bullet_col,
              const sprite::type sprite,
              const float delta_time_s,
              std::vector<Attack>& attacks)
{
  // Ability: Shoot
  // if (keys.shoot_pressed)
  //   obj.bullets_to_fire_after_releasing_mouse_left = obj.bullets_to_fire_after_releasing_mouse;

  // timed bullets
  // if (player.bullet_seconds_between_spawning_left > 0.0f)
  //   player.bullet_seconds_between_spawning_left -= delta_time_s;
  // if (player.bullet_seconds_between_spawning_left <= 0.0f || app.get_input().get_mouse_lmb_down())
  //   player.bullet_seconds_between_spawning_left = player.bullet_seconds_between_spawning;

  if (!app.get_input().get_mouse_lmb_down())
    return;

  // lmb click bullets

  // spawn bullet
  GameObject2D bullet_copy = gameobject::create_bullet(sprite, tex_unit, bullet_col);
  // fix offset issue so bullet spawns in middle of player
  glm::vec2 bullet_pos = fire_from_this_entity.pos;
  bullet_pos.x += fire_from_this_entity.physics_size.x / 2.0f - bullet_copy.physics_size.x / 2.0f;
  bullet_pos.y += fire_from_this_entity.physics_size.y / 2.0f - bullet_copy.physics_size.y / 2.0f;
  bullet_copy.pos = bullet_pos;
  // convert right analogue input to velocity
  bullet_copy.velocity.x = keys.r_analogue_x * bullet_copy.speed_current;
  bullet_copy.velocity.y = keys.r_analogue_y * bullet_copy.speed_current;
  bullets.push_back(bullet_copy);

  // Create an attack ID
  // std::cout << "bullet attack, attack id: " << a.id << std::endl;
  Attack a = Attack(fire_from_this_entity.id, bullet_copy.id, Weapons::PISTOL);
  attacks.push_back(a);
};

// slash stats
const float weapon_radius = 30.0f;
const float slash_attack_time = 0.15f;
float slash_attack_time_left = 0.0f;
float weapon_current_angle = 0.0f;
float weapon_angle_speed = fightingengine::HALF_PI / 30.0f; // closer to 0 is faster
bool attack_left_to_right = true;

void
ability_slash(fightingengine::Application& app,
              GameObject2D& player_obj,
              const KeysAndState& keys,
              GameObject2D& weapon,
              float delta_time_s,
              std::vector<Attack>& attacks)
{
  if (app.get_input().get_mouse_lmb_down()) {
    slash_attack_time_left = slash_attack_time;
    attack_left_to_right = !attack_left_to_right; // keep swapping left to right to right to left etc

    if (attack_left_to_right)
      weapon_current_angle = keys.angle_around_player - fightingengine::HALF_PI / 2.0f;
    else
      weapon_current_angle = keys.angle_around_player + fightingengine::HALF_PI / 2.0f;

    // set angle, but freezes weapon angle throughout slash?
    weapon.angle_radians = keys.angle_around_player + sprite::spritemap::get_sprite_rotation_offset(weapon.sprite);

    // remove any other slash attacks from this player
    std::vector<Attack>::iterator it = attacks.begin();
    while (it != attacks.end()) {
      Attack& att = (*it);
      if (att.entity_weapon_owner_id == player_obj.id && att.weapon_type == Weapons::SHOVEL) {
        it = attacks.erase(it);
      } else {
        ++it;
      }
    }
    // Create a new slash with attack ID
    // std::cout << "slash attack, attack id: " << a.id << std::endl;
    Attack a = Attack(player_obj.id, weapon.id, Weapons::SHOVEL);
    attacks.push_back(a);
  }

  if (slash_attack_time_left > 0.0f) {
    slash_attack_time_left -= delta_time_s;
    weapon.do_render = true;
    weapon.do_physics = true;
  } else {
    weapon.do_render = false;
    weapon.do_physics = false;
  }

  glm::vec2 pos = player_obj.pos;
  pos.x += player_obj.physics_size.x / 2.0f - weapon.physics_size.x / 2.0f;
  pos.y += player_obj.physics_size.y / 2.0f - weapon.physics_size.y / 2.0f;

  if (attack_left_to_right)
    weapon_current_angle += weapon_angle_speed;
  else
    weapon_current_angle -= weapon_angle_speed;

  // offset around center of circle
  glm::vec2 offset_pos =
    glm::vec2(weapon_radius * sin(weapon_current_angle), -weapon_radius * cos(weapon_current_angle));
  weapon.pos = pos + offset_pos;
}

}; // namespace player
}; // namespace game2d
