// your header
#include "player.hpp"

namespace game2d {

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

} // namespace game2d