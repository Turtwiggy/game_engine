#pragma once

// game default states

constexpr int GAME_OVER_WAVE = 10;
constexpr int ENEMY_ATTACK_THRESHOLD = 4000;
constexpr bool SPAWN_ENEMIES = true;
constexpr int EXTRA_ENEMIES_TO_SPAWN_PER_WAVE = 5;
constexpr int SECONDS_UNTIL_MAX_DIFFICULTY = 100;
constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_START = 1.0f;
constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_END = 0.2f;

constexpr int ENEMY_BASE_HEALTH = 6;
constexpr int ENEMY_BASE_DAMAGE = 2;
constexpr int ENEMY_BASE_SPEED = 50;

constexpr float screenshake_time = 0.1f;
constexpr float vfx_flash_time = 0.2f;

// shop & stats
constexpr int shop_refill_pistol_ammo = 5;
constexpr int shop_refill_shotgun_ammo = 5;
constexpr int shop_refill_machinegun_ammo = 5;

// sprites
constexpr game2d::sprite::type sprite_player = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_tree = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_weapon_base = game2d::sprite::type::WEAPON_SHOVEL;
constexpr game2d::sprite::type sprite_pistol = game2d::sprite::type::WEAPON_PISTOL;
constexpr game2d::sprite::type sprite_shotgun = game2d::sprite::type::WEAPON_SHOTGUN;
constexpr game2d::sprite::type sprite_machinegun = game2d::sprite::type::WEAPON_MP5;
constexpr game2d::sprite::type sprite_bullet = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_enemy_core = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_splat = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_heart_1 = game2d::sprite::type::ICON_HEART;
constexpr game2d::sprite::type sprite_heart_2 = game2d::sprite::type::ICON_HEART_OUTLINE;
constexpr game2d::sprite::type sprite_heart_3 = game2d::sprite::type::ICON_HEART_HALF_FULL;
constexpr game2d::sprite::type sprite_heart_4 = game2d::sprite::type::ICON_HEART_FULL;
constexpr game2d::sprite::type sprite_ammo = game2d::sprite::type::AMMO_BOX;

MeleeWeaponStats
create_shovel()
{
  MeleeWeaponStats s;
  s.damage = 1;
  return s;
};

RangedWeaponStats
create_pistol()
{
  RangedWeaponStats r;
  r.damage = 3;
  r.radius_offset_from_player = 14;
  r.infinite_ammo = true;
  r.current_ammo = 0;
  r.fire_rate_seconds_limit = 0.2f;
  return r;
};

RangedWeaponStats
create_shotgun()
{
  RangedWeaponStats r;
  r.damage = 5;
  r.radius_offset_from_player = 17;
  r.infinite_ammo = false;
  r.current_ammo = 0;
  r.fire_rate_seconds_limit = 0.5f;
  return r;
};

RangedWeaponStats
create_machinegun()
{
  RangedWeaponStats r;
  r.damage = 4;
  r.radius_offset_from_player = 16;
  r.infinite_ammo = false;
  r.current_ammo = 0;
  r.fire_rate_seconds_limit = 0.3f;
  return r;
};

// An "Attack" is basically a limiter that prevents collisions
// applying damage on every frame. This could end up being super weird.
struct Attack
{
private:
  static inline uint32_t global_attack_int_counter = 0;

public:
  uint32_t id = 0;

  int entity_weapon_owner_id; // player or enemy
  int entity_weapon_id;
  ShopItem weapon_type;
  int weapon_damage = 0;

  Attack(int parent, int weapon, ShopItem type, int damage)
    : entity_weapon_owner_id(parent)
    , entity_weapon_id(weapon)
    , weapon_type(type)
    , weapon_damage(damage)
  {
    id = ++Attack::global_attack_int_counter;
  };
};

struct WeaponStats
{
  int damage = 0;
};

struct MeleeWeaponStats : WeaponStats
{
  // slash stats
  float slash_attack_time = 0.15f;
  float weapon_radius = 30.0f;
  float weapon_angle_speed = engine::HALF_PI / 30.0f; // closer to 0 is faster
  float weapon_damping = 20.0f;
  float slash_attack_time_left = 0.0f;
  bool attack_left_to_right = true;
  glm::vec2 weapon_target_pos = { 0.0f, 0.0f };
  float weapon_current_angle = 0.0f;
};
struct RangedWeaponStats : WeaponStats
{
  int radius_offset_from_player = 14;
  bool infinite_ammo = true;
  float fire_rate_seconds_limit = 1.0f;
  int current_ammo = 20;
};
void
shop::update_shop(int& p0_currency,
                  int kenny_texture_id,
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

  const glm::vec2 shop_icon_size = { 20.0f, 20.0f };
  for (auto& shop_item : shop) {

    for (auto& icon : shop_item.second.icons) {
      auto uv = convert_sprite_to_uv(icon);
      ImGui::Image((ImTextureID)kenny_texture_id, { shop_icon_size.x, shop_icon_size.y }, uv[0], uv[1]);
      ImGui::SameLine();
    }

    std::string wep = std::string(magic_enum::enum_name(shop_item.first));

    if (shop_item.second.infinite_quantity)
      ImGui::Text("Stock INF Price %i", shop_item.second.price);
    else
      ImGui::Text("Stock %i Price %i", shop_item.second.quantity, shop_item.second.price);

    bool able_to_buy = p0_currency >= shop_item.second.price && shop_item.second.quantity > 0;
    if (able_to_buy) {

      ImGui::SameLine(ImGui::GetWindowWidth() - 40);
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
    }
  }
}

// your header
#include "player.hpp"

glm::ivec2
player::rotate_b_around_a(const GameObject2D& a, const GameObject2D& b, float radius, float angle)
{
  const glm::vec2 center{ 0.0f, 0.0f };
  const glm::vec2 offset = glm::vec2(center.x + radius * cos(angle), center.y + radius * sin(angle));

  // Position b in a circle around the a, and render the b in it's center.
  const glm::vec2 pos = convert_top_left_to_centre(a);
  const glm::vec2 rot_pos = pos + offset - (glm::vec2(b.physics_size) / 2.0f);

  return glm::ivec2(int(rot_pos.x), int(rot_pos.y));
};

void
player::scroll_to_swap_weapons(engine::Application& app, GameObject2D& player, std::vector<ShopItem>& inventory)
{
  float mousewheel = app.get_input().get_mousewheel_y();
  float epsilon = 0.0001f;
  if (mousewheel > epsilon || mousewheel < -epsilon) {
    // int wheel_int = static_cast<int>(mousewheel);
    // std::cout << "wheel int: " << wheel_int << std::endl;
    bool positive_direction = mousewheel > 0;

    // cycle through weapons
    int cur_item_index = player.equipped_item_index;
    if (positive_direction)
      cur_item_index = (cur_item_index + 1) % inventory.size();
    else if (cur_item_index == 0)
      cur_item_index = static_cast<int>(inventory.size() - 1);
    else
      cur_item_index = (cur_item_index - 1) % inventory.size();

    player.equipped_item_index = cur_item_index;
    // std::cout << "equipping item: " << cur_item_index << "mouse was pos: " << positive_direction << std::endl;
  }
};

void
player::ability_boost(GameObject2D& player, const KeysAndState& keys, const float delta_time_s)
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
player::ability_slash(engine::Application& app,
                      GameObject2D& player_obj,
                      KeysAndState& keys,
                      GameObject2D& weapon,
                      float delta_time_s,
                      MeleeWeaponStats& s,
                      std::vector<Attack>& attacks)
{
  if (keys.shoot_down) {
    s.slash_attack_time_left = s.slash_attack_time;
    s.attack_left_to_right = !s.attack_left_to_right; // keep swapping left to right to right to left etc

    if (s.attack_left_to_right)
      s.weapon_current_angle = keys.angle_around_player;
    else
      s.weapon_current_angle = keys.angle_around_player;

    // set angle, but freezes weapon angle throughout slash?
    weapon.angle_radians = keys.angle_around_player;

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

    // Create a new slash with unique attack.id
    Attack a = Attack(player_obj.id, weapon.id, ShopItem::SHOVEL, s.damage);
    attacks.push_back(a);
  }

  if (s.slash_attack_time_left > 0.0f) {
    s.slash_attack_time_left -= delta_time_s;
    weapon.do_render = true;
    weapon.do_physics = true;

    if (s.attack_left_to_right)
      s.weapon_current_angle += s.weapon_angle_speed;
    else
      s.weapon_current_angle -= s.weapon_angle_speed;

    glm::ivec2 pos = rotate_b_around_a(player_obj, weapon, s.weapon_radius, s.weapon_current_angle);
    s.weapon_target_pos = pos;
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
              const glm::vec4 bullet_col,
              const sprite::type sprite,
              RangedWeaponStats& s,
              MutableGameState& gs,
              engine::RandomState& rnd)
{
  s.current_ammo -= 1;
  fire_from_this_entity.flash_time_left = 0.2f;

  // spawn bullet
  GameObject2D bullet_copy = gameobject::create_bullet(sprite, bullet_col);

  // position bullet
  {
    const glm::vec2 center = convert_top_left_to_centre(fire_from_this_entity);
    const glm::vec2 center_corrected = center - (glm::vec2(bullet_copy.physics_size) / 2.0f);

    constexpr glm::vec2 flash_offset = { 16.0f, 16.0f }; // from top left
    const float radius = glm::length(flash_offset);
    const float angle = fire_from_this_entity.angle_radians;

    glm::ivec2 gun_tip = player::rotate_b_around_a(fire_from_this_entity, bullet_copy, radius, angle);
    bullet_copy.pos = gun_tip;

    // spawn a flash
    MuzzleFlashPointLight mfpl;
    mfpl.light_object = gameobject::create_light();
    mfpl.light_object.pos = gun_tip;
    mfpl.light_object.do_lifecycle_timed = true;
    mfpl.light_object.time_alive = 0.3f;
    mfpl.light_object.time_alive_left = 0.3f;
    gs.point_lights.push_back(mfpl);
  }

  // convert right analogue input to velocity
  {
    // bullet_copy.velocity.x = keys.r_analogue_x * bullet_copy.speed_current;
    // bullet_copy.velocity.y = keys.r_analogue_y * bullet_copy.speed_current;
    const float velocity_as_angle = atan2(keys.r_analogue_y, keys.r_analogue_x) + engine::PI;

    // add some randomness to the bullet's direction
    // todo: replace complete randomness with a recoil factor.
    constexpr float angle_min_max = 10.0f * engine::PI / 180.0f;
    const float random_angle = engine::rand_det_s(rnd.rng, -angle_min_max, angle_min_max);

    const float new_angle = velocity_as_angle + random_angle;
    const float rand_x_vel = glm::sin(new_angle);
    const float rand_y_vel = -glm::cos(new_angle);
    bullet_copy.velocity.x = rand_x_vel * bullet_copy.speed_current;
    bullet_copy.velocity.y = rand_y_vel * bullet_copy.speed_current;
  }
  gs.entities_bullets.push_back(bullet_copy);

  // Create an attack ID
  // std::cout << "bullet attack, attack id: " << a.id << std::endl;
  Attack a = Attack(fire_from_this_entity.id, bullet_copy.id, ShopItem::PISTOL, s.damage);
  gs.attacks.push_back(a);
};

void
position_around_player(GameObject2D& player, GameObject2D& weapon, RangedWeaponStats& stats, KeysAndState& keys)
{
  weapon.do_render = true;
  weapon.angle_radians = keys.angle_around_player;

  const int radius = stats.radius_offset_from_player;
  const float angle = keys.angle_around_player;
  glm::vec2 rot_pos = player::rotate_b_around_a(player, weapon, float(radius), angle);
  weapon.pos = rot_pos;
};

void
player::player_attack(engine::Application& app,
                      MutableGameState& gs,
                      GameObject2D& player,
                      std::vector<ShopItem>& player_inventory,
                      KeysAndState& keys,
                      const float delta_time_s,
                      engine::RandomState& rnd)
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
    GameObject2D& weapon = gs.weapon_pistol;
    RangedWeaponStats& stats = gs.stats_pistol;

    position_around_player(player, weapon, stats, keys);

    if ((stats.infinite_ammo || stats.current_ammo > 0) && keys.shoot_down)
      ability_shoot(weapon, keys, bullet_pistol_colour, sprite_bullet, stats, gs, rnd);
  }

  if (player_inventory[player.equipped_item_index] == ShopItem::SHOTGUN) {
    GameObject2D& weapon = gs.weapon_shotgun;
    RangedWeaponStats& stats = gs.stats_shotgun;

    position_around_player(player, weapon, stats, keys);

    if ((stats.infinite_ammo || stats.current_ammo > 0) && keys.shoot_down)
      ability_shoot(weapon, keys, bullet_shotgun_colour, sprite_bullet, stats, gs, rnd);
  }

  if (player_inventory[player.equipped_item_index] == ShopItem::MACHINEGUN) {
    GameObject2D& weapon = gs.weapon_machinegun;
    RangedWeaponStats& stats = gs.stats_machinegun;

    position_around_player(player, weapon, stats, keys);

    if ((stats.infinite_ammo || stats.current_ammo > 0) && keys.shoot_down)
      ability_shoot(weapon, keys, bullet_machinegun_colour, sprite_bullet, stats, gs, rnd);
  }
}

// header
#include "enemy_spawner.hpp"

// other project headers
#include <glm/gtx/norm.hpp>

// game headers
#include "constants.hpp"

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