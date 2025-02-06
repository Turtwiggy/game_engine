#include "manualfire_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/combat_projectiles/projectile_helpers.hpp"
#include "modules/core_colour/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/core_renderer/helpers.hpp"
#include "modules/core_sprites/sprite_helpers.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/system_autofire/autofire_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_hulls/hulls_components.hpp"
#include "modules/system_manualfire/manualfire_components.hpp"


namespace game2d {

void
update_manualfire_system(entt::registry& r)
{
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto& view = r.view<TransformComponent,
                            const WeaponComponent,
                            const HasParentComponent,
                            const HardpointComponent,
                            const ManualfireComponent,
                            CooldownComponent>();

  for (const auto& [wep_e, wep_t, wep_c, parent_c, arc_c, manualfire_c, cooldown_c] : view.each()) {

    const auto p = parent_c.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(wep_e); // kill this parentless entity (soz)
      continue;
    }

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
    s.size = { 16, 16 };
    s.col = parent_col;
    draw_sprite(r, s);

    const auto raw_dir = (tgt_pos - gun_pos);
    const auto nrm_dir = engine::normalize_safe(raw_dir);
    const auto angle = engine::dir_to_angle_radians(nrm_dir);

    // rotate the gun
    const float shoot_angle = engine::dir_to_angle_radians(nrm_dir);
    wep_t.rotation_radians.z = shoot_angle;

    if (cooldown_c.time > 0.0f)
      continue;

    // input
    const bool shoot = parent_input_c.shoot;
    if (!shoot)
      continue;

    reset_cooldown(cooldown_c);

    // TODO: implement same functionality as system autofire
    int bullet_damage = r.get<BulletDamage>(wep_e).damage;
    int bullet_speed = r.get<BulletSpeed>(wep_e).speed;

    BulletDef bullet_def(wep_e);
    bullet_def.key = "bullet_default";
    bullet_def.size = { 6, 6 };
    bullet_def.team = AvailableTeams::player;
    bullet_def.damage = bullet_damage;
    bullet_def.speed = bullet_speed;
    bullet_def.lifecycle = 3 * 1000;

    const auto pos = gun_pos;
    const auto bullet_e = spawn_projectile(r, bullet_def, pos);

    // set velocity
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    body_c.body->SetLinearVelocity(b2Vec2{ bullet_speed * nrm_dir.x, bullet_speed * nrm_dir.y });
  }
}

} // namespace game2d