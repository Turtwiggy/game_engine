
// your header
#include "2d_game_logic.hpp"

// c++ lib headers
#include <iostream>
#include <vector>

// other lib headers
#include "thirdparty/magic_enum.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp> // for distance2
#include <imgui.h>

// engine headers
#include "engine/maths_core.hpp"

// game2d headers
#include "2d_game_config.hpp"

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
camera::update(GameObject2D& camera, KeysAndState& keys, fightingengine::Application& app, float delta_time_s)
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

void
next_wave(int& enemies_to_spawn_this_wave, int& enemies_to_spawn_this_wave_left, int& wave)
{
  enemies_to_spawn_this_wave += EXTRA_ENEMIES_TO_SPAWN_PER_WAVE * wave;
  enemies_to_spawn_this_wave_left = enemies_to_spawn_this_wave;
  wave += 1;
  std::cout << "left: " << enemies_to_spawn_this_wave_left << std::endl;
}

// TODO fix this
void
spawn_enemy(MutableGameState& state, fightingengine::RandomState& rnd, glm::vec2 world_pos)
{
  const int wave = state.wave;

  // spawn enemy
  GameObject2D enemy_copy = gameobject::create_enemy(rnd);
  // override defaults
  enemy_copy.pos = world_pos;

  // override stats based on wave
  int base_health = 6;
  int base_damage = 2;
  int base_speed = 50;

  if (wave < 28) {
    enemy_copy.damage_able_to_be_taken = static_cast<int>(base_health * wave_toughness[wave]); // toughness
    enemy_copy.damage_to_give_player = static_cast<int>(base_damage * wave_damage[wave]);      // damage
  } else {
    std::cout << " This is the last (curated) wave...!" << std::endl;
    enemy_copy.damage_able_to_be_taken = static_cast<int>(base_health * 25.0f); // toughness
    enemy_copy.damage_to_give_player = static_cast<int>(base_damage * 25.0f);   // damage
  }

  if (wave < 15)
    enemy_copy.speed_current = base_speed * wave_speed[wave]; // speed
  else
    enemy_copy.speed_current = 3.5f;

  state.entities_enemies.push_back(enemy_copy);
}

void
update(MutableGameState& state, fightingengine::RandomState& rnd, const glm::ivec2 screen_wh, const float delta_time_s)
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

      for (auto& player : state.entities_player) {

        distance_squared = glm::distance2(rnd_pos, player.pos);
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

    if (game_spawn_enemies && state.enemies_to_spawn_this_wave_left > 0) {
      spawn_enemy(state, rnd, world_pos);
      state.enemies_to_spawn_this_wave_left -= 1;
    }
  }

  // increase difficulty
  // 0.5 is starting cooldown
  // after 30 seconds, cooldown should be 0
  state.game_seconds_until_max_difficulty_spent += delta_time_s;
  float percent =
    glm::clamp(state.game_seconds_until_max_difficulty_spent / game_seconds_until_max_difficulty, 0.0f, 1.0f);
  state.game_enemy_seconds_between_spawning_current =
    glm::mix(game_enemies_seconds_between_spawning_start, game_enemy_seconds_between_spawning_end, percent);
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
ability_slash(fightingengine::Application& app,
              GameObject2D& player_obj,
              KeysAndState& keys,
              GameObject2D& weapon,
              float delta_time_s,
              MeleeWeaponStats& s,
              std::vector<Attack>& attacks)
{
  if (app.get_input().get_mouse_lmb_down()) {
    s.slash_attack_time_left = s.slash_attack_time;
    s.attack_left_to_right = !s.attack_left_to_right; // keep swapping left to right to right to left etc

    if (s.attack_left_to_right)
      s.weapon_current_angle = keys.angle_around_player - fightingengine::HALF_PI / 2.0f;
    else
      s.weapon_current_angle = keys.angle_around_player + fightingengine::HALF_PI / 2.0f;

    // set angle, but freezes weapon angle throughout slash?
    weapon.angle_radians = keys.angle_around_player + sprite::spritemap::get_sprite_rotation_offset(weapon.sprite);

    // remove any other slash attacks from this player
    std::vector<Attack>::iterator it = attacks.begin();
    while (it != attacks.end()) {
      Attack& att = (*it);
      if (att.entity_weapon_owner_id == player_obj.id && att.weapon_type == ShopItem::SHOVEL) {
        it = attacks.erase(it);
      } else {
        ++it;
      }
    }
    // Create a new slash with attack ID
    // std::cout << "slash attack, attack id: " << a.id << std::endl;
    Attack a = Attack(player_obj.id, weapon.id, ShopItem::SHOVEL, s.damage);
    attacks.push_back(a);
  }

  if (s.slash_attack_time_left > 0.0f) {
    s.slash_attack_time_left -= delta_time_s;
    weapon.do_render = true;
    weapon.do_physics = true;

    glm::vec2 pos = player_obj.pos;
    pos.x += player_obj.physics_size.x / 2.0f - weapon.physics_size.x / 2.0f;
    pos.y += player_obj.physics_size.y / 2.0f - weapon.physics_size.y / 2.0f;

    if (s.attack_left_to_right)
      s.weapon_current_angle += s.weapon_angle_speed;
    else
      s.weapon_current_angle -= s.weapon_angle_speed;

    // offset around center of circle
    glm::vec2 offset_pos =
      glm::vec2(s.weapon_radius * sin(s.weapon_current_angle), -s.weapon_radius * cos(s.weapon_current_angle));
    s.weapon_target_pos = pos + offset_pos;
  } else {
    weapon.do_physics = false;
    s.weapon_target_pos = player_obj.pos;
  }

  // lerp weapon to target position
  weapon.pos = glm::lerp(glm::vec3(weapon.pos.x, weapon.pos.y, 0.0f),
                         glm::vec3(s.weapon_target_pos.x, s.weapon_target_pos.y, 0.0f),
                         glm::clamp(delta_time_s * s.weapon_damping, 0.0f, 1.0f));
}

void
ability_shoot(GameObject2D& fire_from_this_entity,
              KeysAndState& keys,
              std::vector<GameObject2D>& bullets,
              const glm::vec4 bullet_col,
              const sprite::type sprite,
              RangedWeaponStats& s,
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

  s.current_ammo -= 1;
  fire_from_this_entity.flash_time_left = 0.2f;

  // lmb click bullets

  // spawn bullet
  GameObject2D bullet_copy = gameobject::create_bullet(sprite, bullet_col);
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
  Attack a = Attack(fire_from_this_entity.id, bullet_copy.id, ShopItem::PISTOL, s.damage);
  attacks.push_back(a);
};

void
player_attack(fightingengine::Application& app,
              GameObject2D& player,
              MutableGameState& gs,
              std::vector<ShopItem>& player_inventory,
              KeysAndState& keys,
              const float delta_time_s)
{
  gs.weapon_shovel.do_render = false;
  gs.weapon_pistol.do_render = false;
  gs.weapon_shotgun.do_render = false;
  gs.weapon_machinegun.do_render = false;

  if (player_inventory[player.equipped_item_index] == ShopItem::SHOVEL) {
    gs.weapon_shovel.do_render = true;
    ability_slash(app, player, keys, gs.weapon_shovel, delta_time_s, gs.stats_shovel, gs.attacks);
  }

  if (player_inventory[player.equipped_item_index] == ShopItem::PISTOL) {
    gs.weapon_pistol.do_render = true;
    float angle_around_player = keys.angle_around_player;
    glm::vec2 offset = glm::vec2(gs.stats_pistol.radius_offset_from_player * sin(angle_around_player),
                                 -gs.stats_pistol.radius_offset_from_player * cos(angle_around_player));
    gs.weapon_pistol.pos = player.pos + offset;
    gs.weapon_pistol.angle_radians =
      keys.angle_around_player + sprite::spritemap::get_sprite_rotation_offset(gs.weapon_pistol.sprite);

    if (gs.stats_pistol.infinite_ammo || gs.stats_pistol.current_ammo > 0) {
      if (app.get_input().get_mouse_lmb_down()) {
        ability_shoot(gs.weapon_pistol,
                      keys,
                      gs.entities_bullets,
                      bullet_pistol_colour,
                      sprite_bullet,
                      gs.stats_pistol,
                      gs.attacks);
      }
    }
  }

  if (player_inventory[player.equipped_item_index] == ShopItem::SHOTGUN) {
    gs.weapon_shotgun.do_render = true;
    float angle_around_player = keys.angle_around_player;
    glm::vec2 offset = glm::vec2(gs.stats_shotgun.radius_offset_from_player * sin(angle_around_player),
                                 -gs.stats_shotgun.radius_offset_from_player * cos(angle_around_player));
    gs.weapon_shotgun.pos = player.pos + offset;
    gs.weapon_shotgun.angle_radians =
      keys.angle_around_player + sprite::spritemap::get_sprite_rotation_offset(gs.weapon_shotgun.sprite);

    if (gs.stats_shotgun.infinite_ammo || gs.stats_shotgun.current_ammo > 0) {
      if (app.get_input().get_mouse_lmb_down()) {
        ability_shoot(gs.weapon_shotgun,
                      keys,
                      gs.entities_bullets,
                      bullet_pistol_colour,
                      sprite_bullet,
                      gs.stats_shotgun,
                      gs.attacks);
      }
    }
  }

  if (player_inventory[player.equipped_item_index] == ShopItem::MACHINEGUN) {
    gs.weapon_machinegun.do_render = true;
    float angle_around_player = keys.angle_around_player;
    glm::vec2 offset = glm::vec2(gs.stats_machinegun.radius_offset_from_player * sin(angle_around_player),
                                 -gs.stats_machinegun.radius_offset_from_player * cos(angle_around_player));
    gs.weapon_machinegun.pos = player.pos + offset;
    gs.weapon_machinegun.angle_radians =
      keys.angle_around_player + sprite::spritemap::get_sprite_rotation_offset(gs.weapon_machinegun.sprite);

    if (gs.stats_machinegun.infinite_ammo || gs.stats_machinegun.current_ammo > 0) {
      if (app.get_input().get_mouse_lmb_down()) {
        ability_shoot(gs.weapon_machinegun,
                      keys,
                      gs.entities_bullets,
                      bullet_machinegun_colour,
                      sprite_bullet,
                      gs.stats_machinegun,
                      gs.attacks);
      }
    }
  }
}

}; // namespace player

namespace shop {

std::map<ShopItem, ShopItemState>
shop_initial_state()
{
  // configure shop
  std::map<ShopItem, ShopItemState> shop;
  {
    {
      ShopItemState i;
      i.price = 10;
      i.quantity = 1;
      i.infinite_quantity = false;
      shop[ShopItem::PISTOL] = i;
    };
    {
      ShopItemState i;
      i.price = 3;
      i.infinite_quantity = true;
      shop[ShopItem::PISTOL_AMMO] = i;
    };
    {
      ShopItemState i;
      i.price = 20;
      i.quantity = 1;
      i.infinite_quantity = false;
      shop[ShopItem::SHOTGUN] = i;
    };
    {
      ShopItemState i;
      i.price = 6;
      i.infinite_quantity = true;
      shop[ShopItem::SHOTGUN_AMMO] = i;
    };
    {
      ShopItemState i;
      i.price = 40;
      i.quantity = 1;
      i.infinite_quantity = false;
      shop[ShopItem::MACHINEGUN] = i;
    };
    {
      ShopItemState i;
      i.price = 5;
      i.infinite_quantity = true;
      shop[ShopItem::MACHINEGUN_AMMO] = i;
    };
    {
      ShopItemState i;
      i.price = 40;
      i.infinite_quantity = true;
      shop[ShopItem::HEAL_HALF] = i;
    };
    {
      ShopItemState i;
      i.price = 80;
      i.infinite_quantity = true;
      shop[ShopItem::HEAL_FULL] = i;
    };
  };
  return shop;
}

void
update_shop(int& p0_currency,
            std::map<ShopItem, ShopItemState>& shop,
            RangedWeaponStats& stats_pistol,
            RangedWeaponStats& stats_shotgun,
            RangedWeaponStats& stats_machinegun,
            int shop_refill_pistol_ammo,
            int shop_refill_shotgun_ammo,
            int shop_refill_machinegun_ammo,
            std::vector<std::vector<ShopItem>>& player_inventories,
            std::vector<GameObject2D>& entities_player)
{
  ImGui::Text("You have %i coin!", p0_currency);

  if (ImGui::Button("Drain your coin..."))
    p0_currency -= 1;

  for (auto& shop_item : shop) {

    std::string wep = std::string(magic_enum::enum_name(shop_item.first));

    bool able_to_buy = p0_currency >= shop_item.second.price && shop_item.second.quantity > 0;
    if (able_to_buy) {
      std::string buy_button_label = "Buy ##" + wep;
      bool buy_button_clicked = ImGui::Button(buy_button_label.c_str());
      if (buy_button_clicked) {
        std::cout << "buy: " << wep << " clicked" << std::endl;

        // reduce item quantity if not infinite
        if (!shop_item.second.infinite_quantity)
          shop_item.second.quantity -= 1;

        // spend hard earned cash
        p0_currency -= shop_item.second.price;

        // shop logic
        {
          if (shop_item.first == ShopItem::PISTOL || shop_item.first == ShopItem::SHOTGUN ||
              shop_item.first == ShopItem::MACHINEGUN) {
            // hack: use player 0 for the moment
            std::vector<ShopItem>& player_inv = player_inventories[0];
            player_inv.push_back(shop_item.first);
          }

          if (shop_item.first == ShopItem::PISTOL_AMMO)
            stats_pistol.current_ammo += shop_refill_pistol_ammo;
          if (shop_item.first == ShopItem::SHOTGUN_AMMO)
            stats_shotgun.current_ammo += shop_refill_shotgun_ammo;
          if (shop_item.first == ShopItem::MACHINEGUN_AMMO)
            stats_machinegun.current_ammo += shop_refill_machinegun_ammo;

          if (shop_item.first == ShopItem::HEAL_HALF) {
            GameObject2D& p0 = entities_player[0];
            p0.damage_taken -= static_cast<int>(p0.damage_able_to_be_taken / 2);
            if (p0.damage_taken < 0)
              p0.damage_taken = 0;
          }

          if (shop_item.first == ShopItem::HEAL_FULL) {
            GameObject2D& p0 = entities_player[0];
            p0.damage_taken = 0;
          }
        }
      }
      ImGui::SameLine();
    }

    ImGui::Text("Item: %s Quantiy: %i Price: %i", wep.c_str(), shop_item.second.quantity, shop_item.second.price);
  }
}

}; // namespace shop

}; // namespace game2d
