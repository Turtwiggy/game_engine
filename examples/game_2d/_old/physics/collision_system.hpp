#pragma once

// { // Resolve collision events

//   for (auto& event : gs.collision_events) {

//     auto& coll_layer_0 = event.go0.collision_layer;
//     auto& coll_layer_1 = event.go1.collision_layer;

//     if ((coll_layer_0 == CollisionLayer::Player && coll_layer_1 == CollisionLayer::Enemy) ||
//         (coll_layer_1 == CollisionLayer::Player && coll_layer_0 == CollisionLayer::Enemy)) {

//       GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Enemy ? event.go0 : event.go1;
//       GameObject2D& player = event.go0.collision_layer == CollisionLayer::Enemy ? event.go1 : event.go0;

//       if (player.damage_taken >= player.damage_able_to_be_taken)
//         continue; // player is dead

//       enemy.flag_for_delete = true;                       // enemy
//       player.damage_taken += enemy.damage_to_give_player; // player
//       player.flash_time_left = vfx_flash_time;            // vfx: flash
//       auto& ss_timer = registry.ctx<ScreenShakeTimer>();
//       ss_timer.time_left = screenshake_time; // screenshake

//       // vfx spawn a splat
//       GameObject2D splat = gameobject::create_generic();
//       splat.sprite = sprite_splat;
//       splat.colour = player_splat_colour;
//       splat.pos = player.pos;
//       splat.angle_radians = engine::rand_det_s(rnd.rng, 0.0f, engine::PI);
//       gs.entities_vfx.push_back(splat);
//     }

//     if ((coll_layer_0 == CollisionLayer::Enemy && coll_layer_1 == CollisionLayer::Weapon) ||
//         (coll_layer_1 == CollisionLayer::Enemy && coll_layer_0 == CollisionLayer::Weapon)) {

//       GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Enemy ? event.go0 : event.go1;
//       GameObject2D& weapon = event.go0.collision_layer == CollisionLayer::Enemy ? event.go1 : event.go0;
//       GameObject2D& player = gs.entities_player[0]; // hack: use player 0 for the moment

//       for (auto& attack : gs.attacks) {

//         bool is_shovel = attack.weapon_type == ShopItem::SHOVEL;
//         bool collision_with_specific_shovel_attack = weapon.id == attack.entity_weapon_id;
//         bool taken_damage_from_shovel =
//           std::find(enemy.attack_ids_taken_damage_from.begin(), enemy.attack_ids_taken_damage_from.end(), attack.id)
//           != enemy.attack_ids_taken_damage_from.end();

//         if (is_shovel && collision_with_specific_shovel_attack && !taken_damage_from_shovel) {
//           // std::cout << "enemy taking damage from weapon attack ONCE!" << std::endl;
//           enemy.damage_taken += attack.weapon_damage;
//           enemy.attack_ids_taken_damage_from.push_back(attack.id);
//           enemy.flash_time_left = vfx_flash_time; // vfx: flash

//           // vfx impact splat
//           int damage_amount = attack.weapon_damage;
//           vfx::spawn_impact_splats(rnd, player, enemy, sprite_splat, damage_amount, gs.entities_vfx);
//         }
//       }
//     }

//     if ((coll_layer_0 == CollisionLayer::Bullet && coll_layer_1 == CollisionLayer::Enemy) ||
//         (coll_layer_1 == CollisionLayer::Bullet && coll_layer_0 == CollisionLayer::Enemy)) {
//       GameObject2D& bullet = event.go0.collision_layer == CollisionLayer::Bullet ? event.go0 : event.go1;
//       GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Bullet ? event.go1 : event.go0;
//       GameObject2D& player = gs.entities_player[0]; // hack: use player 0 for the moment

//       for (auto& attack : gs.attacks) {

//         bool is_bullet = attack.weapon_type == ShopItem::PISTOL;
//         bool collision_with_specific_bullet = bullet.id == attack.entity_weapon_id;
//         bool taken_damage_from_bullet =
//           std::find(enemy.attack_ids_taken_damage_from.begin(), enemy.attack_ids_taken_damage_from.end(), attack.id)
//           != enemy.attack_ids_taken_damage_from.end();

//         if (is_bullet && collision_with_specific_bullet && !taken_damage_from_bullet) {
//           // std::cout << "enemy taking damage from bullet attack ONCE!" << std::endl;
//           enemy.damage_taken += attack.weapon_damage;
//           enemy.attack_ids_taken_damage_from.push_back(attack.id);
//           enemy.flash_time_left = vfx_flash_time; // vfx: flash

//           // vfx impactsplat
//           int damage_amount = attack.weapon_damage;
//           vfx::spawn_impact_splats(rnd, player, enemy, sprite_splat, damage_amount, gs.entities_vfx);
//         }
//       }
//     }

//     if ((coll_layer_0 == CollisionLayer::Obstacle && coll_layer_1 == CollisionLayer::Player) ||
//         (coll_layer_1 == CollisionLayer::Obstacle && coll_layer_0 == CollisionLayer::Player)) {

//       GameObject2D& obstacle = event.go0.collision_layer == CollisionLayer::Obstacle ? event.go0 : event.go1;
//       GameObject2D& player = event.go0.collision_layer == CollisionLayer::Obstacle ? event.go1 : event.go0;

//       glm::ivec2 obstacle_pos = convert_top_left_to_centre(obstacle);
//       glm::ivec2 player_pos = convert_top_left_to_centre(player);

//       // push the obstacle in x, -x, y, or -y i.e. individual axis
//       glm::vec2 dir = glm::normalize(glm::vec2(player_pos - obstacle_pos));
//       if (glm::abs(dir.x) > glm::abs(dir.y)) {
//         if (dir.x < 0) {
//           obstacle.pos.x += 1;
//         } else {
//           obstacle.pos.x -= 1;
//         }
//       } else {
//         if (dir.y < 0) {
//           obstacle.pos.y += 1;
//         } else {
//           obstacle.pos.y -= 1;
//         }
//       }

//       // ImGui::Begin("Huh. Well then.", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
//       // ImGui::Text("You are standing at a tree. Cool!");
//       // ImGui::End();
//     }

//     if ((coll_layer_0 == CollisionLayer::Obstacle && coll_layer_1 == CollisionLayer::Enemy) ||
//         (coll_layer_1 == CollisionLayer::Obstacle && coll_layer_0 == CollisionLayer::Enemy)) {
//       GameObject2D& obstacle = event.go0.collision_layer == CollisionLayer::Obstacle ? event.go0 : event.go1;
//       GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Obstacle ? event.go1 : event.go0;

//       // std::cout << "enemy taking damage from bullet attack ONCE!" << std::endl;
//       const int DAMAGE_TO_GIVE_ENEMY_FROM_OBSTACLE = 10;
//       enemy.damage_taken += DAMAGE_TO_GIVE_ENEMY_FROM_OBSTACLE;
//       enemy.flash_time_left = vfx_flash_time;

//       const int DAMAGE_TO_GIVE_OBSTACLE_FROM_ENEMY = 1;
//       obstacle.damage_taken += DAMAGE_TO_GIVE_OBSTACLE_FROM_ENEMY;
//       obstacle.flash_time_left = vfx_flash_time;

//       // vfx impactsplat
//       int damage_amount = DAMAGE_TO_GIVE_ENEMY_FROM_OBSTACLE;
//       vfx::spawn_impact_splats(rnd, enemy, obstacle, sprite_splat, damage_amount, gs.entities_vfx);
//     }
//   }
// }
