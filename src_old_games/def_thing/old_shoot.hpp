
// void
// ability_shoot(GameObject2D& fire_from_this_entity,
//               KeysAndState& keys,
//               const glm::vec4 bullet_col,
//               const sprite::type sprite,
//               RangedWeaponStats& s,
//               MutableGameState& gs,
//               engine::RandomState& rnd)
// {
//   s.current_ammo -= 1;
//   fire_from_this_entity.flash_time_left = 0.2f;

//   // spawn bullet
//   GameObject2D bullet_copy = gameobject::create_bullet(sprite, bullet_col);

//   // position bullet
//   {
//     const glm::vec2 center = convert_top_left_to_centre(fire_from_this_entity);
//     const glm::vec2 center_corrected = center - (glm::vec2(bullet_copy.physics_size) / 2.0f);

//     constexpr glm::vec2 flash_offset = { 16.0f, 16.0f }; // from top left
//     const float radius = glm::length(flash_offset);
//     const float angle = fire_from_this_entity.angle_radians;

//     glm::ivec2 gun_tip = player::rotate_b_around_a(fire_from_this_entity, bullet_copy, radius, angle);
//     bullet_copy.pos = gun_tip;

//     // spawn a flash
//     MuzzleFlashPointLight mfpl;
//     mfpl.light_object = gameobject::create_light();
//     mfpl.light_object.pos = gun_tip;
//     mfpl.light_object.do_lifecycle_timed = true;
//     mfpl.light_object.time_alive = 0.3f;
//     mfpl.light_object.time_alive_left = 0.3f;
//     gs.point_lights.push_back(mfpl);
//   }

//   // convert right analogue input to velocity
//   {
//     // bullet_copy.velocity.x = keys.r_analogue_x * bullet_copy.speed_current;
//     // bullet_copy.velocity.y = keys.r_analogue_y * bullet_copy.speed_current;
//     const float velocity_as_angle = atan2(keys.r_analogue_y, keys.r_analogue_x) + engine::PI;

//     // add some randomness to the bullet's direction
//     // todo: replace complete randomness with a recoil factor.
//     constexpr float angle_min_max = 10.0f * engine::PI / 180.0f;
//     const float random_angle = engine::rand_det_s(rnd.rng, -angle_min_max, angle_min_max);

//     const float new_angle = velocity_as_angle + random_angle;
//     const float rand_x_vel = glm::sin(new_angle);
//     const float rand_y_vel = -glm::cos(new_angle);
//     bullet_copy.velocity.x = rand_x_vel * bullet_copy.speed_current;
//     bullet_copy.velocity.y = rand_y_vel * bullet_copy.speed_current;
//   }
//   gs.entities_bullets.push_back(bullet_copy);

//   // Create an attack ID
//   // std::cout << "bullet attack, attack id: " << a.id << std::endl;
//   Attack a = Attack(fire_from_this_entity.id, bullet_copy.id, ShopItem::PISTOL, s.damage);
//   gs.attacks.push_back(a);
// };