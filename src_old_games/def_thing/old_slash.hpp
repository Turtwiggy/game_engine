
// void
// player::player_attack(engine::Application& app,
//                       MutableGameState& gs,
//                       GameObject2D& player,
//                       std::vector<ShopItem>& player_inventory,
//                       KeysAndState& keys,
//                       const float delta_time_s,
//                       engine::RandomState& rnd)
// {
//   gs.weapon_shovel.do_render = false;
//   gs.weapon_pistol.do_render = false;
//   gs.weapon_shotgun.do_render = false;
//   gs.weapon_machinegun.do_render = false;

//   if (player_inventory[player.equipped_item_index] == ShopItem::SHOVEL) {
//     gs.weapon_shovel.do_render = true;
//     ability_slash(app, player, keys, gs.weapon_shovel, delta_time_s, gs.stats_shovel, gs.attacks);
//   }

//   if (player_inventory[player.equipped_item_index] == ShopItem::PISTOL) {
//     GameObject2D& weapon = gs.weapon_pistol;
//     RangedWeaponStats& stats = gs.stats_pistol;

//     position_around_player(player, weapon, stats, keys);

//     if ((stats.infinite_ammo || stats.current_ammo > 0) && keys.shoot_down)
//       ability_shoot(weapon, keys, bullet_pistol_colour, sprite_bullet, stats, gs, rnd);
//   }

//   if (player_inventory[player.equipped_item_index] == ShopItem::SHOTGUN) {
//     GameObject2D& weapon = gs.weapon_shotgun;
//     RangedWeaponStats& stats = gs.stats_shotgun;

//     position_around_player(player, weapon, stats, keys);

//     if ((stats.infinite_ammo || stats.current_ammo > 0) && keys.shoot_down)
//       ability_shoot(weapon, keys, bullet_shotgun_colour, sprite_bullet, stats, gs, rnd);
//   }

//   if (player_inventory[player.equipped_item_index] == ShopItem::MACHINEGUN) {
//     GameObject2D& weapon = gs.weapon_machinegun;
//     RangedWeaponStats& stats = gs.stats_machinegun;

//     position_around_player(player, weapon, stats, keys);

//     if ((stats.infinite_ammo || stats.current_ammo > 0) && keys.shoot_down)
//       ability_shoot(weapon, keys, bullet_machinegun_colour, sprite_bullet, stats, gs, rnd);
//   }
// }

// void
// player::ability_slash(engine::Application& app,
//                       GameObject2D& player_obj,
//                       KeysAndState& keys,
//                       GameObject2D& weapon,
//                       float delta_time_s,
//                       MeleeWeaponStats& s,
//                       std::vector<Attack>& attacks)
// {
//   if (keys.shoot_down) {
//     s.slash_attack_time_left = s.slash_attack_time;
//     s.attack_left_to_right = !s.attack_left_to_right; // keep swapping left to right to right to left etc

//     if (s.attack_left_to_right)
//       s.weapon_current_angle = keys.angle_around_player;
//     else
//       s.weapon_current_angle = keys.angle_around_player;

//     // set angle, but freezes weapon angle throughout slash?
//     weapon.angle_radians = keys.angle_around_player;

//     // remove any other slash attacks from this player
//     std::vector<Attack>::iterator it = attacks.begin();
//     while (it != attacks.end()) {
//       Attack& att = (*it);
//       if (att.entity_weapon_owner_id == player_obj.id && att.weapon_type == ShopItem::SHOVEL) {
//         it = attacks.erase(it);
//       } else {
//         ++it;
//       }
//     }

//     // Create a new slash with unique attack.id
//     Attack a = Attack(player_obj.id, weapon.id, ShopItem::SHOVEL, s.damage);
//     attacks.push_back(a);
//   }

//   if (s.slash_attack_time_left > 0.0f) {
//     s.slash_attack_time_left -= delta_time_s;
//     weapon.do_render = true;
//     weapon.do_physics = true;

//     if (s.attack_left_to_right)
//       s.weapon_current_angle += s.weapon_angle_speed;
//     else
//       s.weapon_current_angle -= s.weapon_angle_speed;

//     glm::ivec2 pos = rotate_b_around_a(player_obj, weapon, s.weapon_radius, s.weapon_current_angle);
//     s.weapon_target_pos = pos;
//   } else {
//     weapon.do_physics = false;
//     s.weapon_target_pos = player_obj.pos;
//   }

//   // lerp weapon to target position
//   weapon.pos = glm::lerp(glm::vec3(weapon.pos.x, weapon.pos.y, 0.0f),
//                          glm::vec3(s.weapon_target_pos.x, s.weapon_target_pos.y, 0.0f),
//                          glm::clamp(delta_time_s * s.weapon_damping, 0.0f, 1.0f));
// }