#include "manualfire_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/combat/combat_projectiles/projectile_helpers.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/events/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/systems/system_autofire/autofire_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_cooldown/helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_manualfire/manualfire_components.hpp"

namespace game2d {

void
update_manualfire_system(entt::registry& r, const float dt)
{
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto& view = r.view<TransformComponent,
                            const WeaponComponent,
                            const HasParentComponent,
                            const HardpointComponent,
                            const ManualfireComponent,
                            WeaponClipSize,
                            WeaponFireRate,
                            WeaponReloadRate,
                            WeaponRange>();

  for (const auto& [wep_e,
                    wep_t,
                    wep_c,
                    parent_c,
                    arc_c,
                    manualfire_c,
                    weapon_clip_size_c,
                    weapon_fire_rate_c,
                    weapon_reload_rate_c,
                    weapon_range_c] : view.each()) {

    const auto p = parent_c.parent;

    // Get modded weapon values.
    const auto wep_def = get_weapon_def(r, p, wep_e);

    const auto& parent_t = r.get<TransformComponent>(p);
    const auto& parent_col = r.get<DefaultColour>(p).colour;
    const auto& parent_input_c = r.get<InputComponent>(p);

    const auto you_pos = glm::vec2{ parent_t.position.x, parent_t.position.y };
    const auto gun_pos = glm::vec2{ wep_t.position.x, wep_t.position.y };
    const auto input = glm::vec2{ parent_input_c.rx, parent_input_c.ry };

    const float radius = 40.0f;
    const auto tgt_pos = you_pos + radius * input;

    // draw a crosshair at your target direction
    Sprite s;
    s.pos = tgt_pos;
    s.sprite = "CROSSHAIR_1";
    s.size = (glm::dot(input, input) > 0) ? glm::vec2{ 16, 16 } : glm::vec2{ 0.0f, 0.0f };
    s.col = parent_col;
    draw_sprite(r, s);

    // if the weapon is reloading, just do that.
    if (weapon_reload_rate_c.seconds_cur > 0.0) {
      weapon_reload_rate_c.seconds_cur -= dt;
      continue;
    }

    // you've reloaded
    if (weapon_clip_size_c.bullets_cur <= 0)
      weapon_clip_size_c.bullets_cur = wep_def.bullets_max;

    const auto raw_dir = (tgt_pos - gun_pos);
    const auto nrm_dir = engine::normalize_safe(raw_dir);
    const auto angle = engine::dir_to_angle_radians(nrm_dir);

    // rotate the gun
    const float shoot_angle = engine::dir_to_angle_radians(nrm_dir);
    wep_t.rotation_radians.z = shoot_angle;

    // Check if you're fire-rate limited.
    // note: updates the _max time based on the modded firerate
    weapon_fire_rate_c.seconds_between_shots_max = 1.0 / wep_def.fire_rate;
    if (weapon_fire_rate_c.seconds_between_shots_left >= 0.0) {
      weapon_fire_rate_c.seconds_between_shots_left -= dt;
      continue;
    }

    // Check the clip size.
    if (weapon_clip_size_c.bullets_cur <= 0) { // time to reload
      weapon_reload_rate_c.seconds_cur = wep_def.reload_rate;
      continue;
    }

    // Check if you need to reload.
    if (weapon_clip_size_c.bullets_cur <= 0)
      weapon_reload_rate_c.seconds_cur = wep_def.reload_rate;

    // input
    const bool shoot = false;
    // parent_input_c. shoot;
    if (!shoot)
      continue;

    // Shoot a bullet! (which can be multiple projectiles)
    weapon_clip_size_c.bullets_cur--;
    weapon_fire_rate_c.seconds_between_shots_left = weapon_fire_rate_c.seconds_between_shots_max;

    // play audio
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "SHOOT_0" });

    // TODO: implement same functionality as system autofire
    const int bullet_damage = r.get<BulletDamage>(wep_e).damage;
    // const int bullet_speed = r.get<BulletSpeed>(wep_e).speed;

    BulletDef bullet_def;
    bullet_def.key = "bullet_default";
    bullet_def.parent_e = wep_e;
    bullet_def.size = { 6, 4 };
    bullet_def.team = AvailableTeams::player;
    bullet_def.damage = bullet_damage;
    bullet_def.speed = 0.05f; // m/s
    bullet_def.lifecycle = 3 * 1000;

    const auto pos = gun_pos;
    const auto bullet_e = spawn_projectile(r, bullet_def, pos);

    // HACK: overwrite manual-bullet sprite
    // set_sprite(r, bullet_e, "EFFECT_29_11");

    // Get your current velocity
    const auto& p_body = r.get<PhysicsBodyComponent>(p).body;
    const auto& p_vel = p_body->GetLinearVelocity();

    // set velocity (add parents velocity)
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    const auto bullet_vel = bullet_def.speed * b2Vec2{ nrm_dir.x, nrm_dir.y };
    body_c.body->ApplyLinearImpulseToCenter(bullet_vel, true);
  }
}

} // namespace game2d