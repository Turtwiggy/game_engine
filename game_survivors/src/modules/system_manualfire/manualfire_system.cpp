#include "manualfire_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/event_coll_bullet_enemy/event_coll_bullet_enemy_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/sprites/sprite_helpers.hpp"
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

    // input
    const auto tgt_pos = you_pos + 20.0f * glm::vec2{ parent_input_c.rx, parent_input_c.ry };
    const bool shoot = parent_input_c.shoot;

    // draw a crosshair at your target direction
    Sprite s;
    s.pos = gun_pos + 20.0f * glm::vec2{ parent_input_c.rx, parent_input_c.ry };
    s.sprite = "CROSSHAIR_1";
    s.size = { 16, 16 };
    s.col = parent_col;
    draw_sprite(r, s);

    if (cooldown_c.time > 0.0f)
      continue;

    const auto raw_dir = tgt_pos - you_pos;
    const auto nrm_dir = engine::normalize_safe(raw_dir);
    const auto angle = engine::dir_to_angle_radians(nrm_dir);

    // rotate the gun
    const float shoot_angle = engine::dir_to_angle_radians(nrm_dir);
    wep_t.rotation_radians.z = shoot_angle;

    if (!shoot)
      continue;
    reset_cooldown(cooldown_c);

    int bullet_damage = r.get<BulletDamage>(parent_c.parent).dmg;

    auto bullet_e = spawn(r, "bullet_default");
    give_life(r, bullet_e, get_position(r, wep_e), { 6, 6 });
    r.emplace<TeamComponent>(bullet_e, AvailableTeams::player);
    r.emplace<BulletComponent>(bullet_e, bullet_damage);
    r.get<PhysicsBodyComponent>(bullet_e).base_speed = 250.0f;
    r.emplace<EntityTimedLifecycle>(bullet_e, 3 * 1000);
    set_z_index(r, bullet_e, ZLayer::PROJECTILE);

    // set velocity
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    body_c.body->SetLinearVelocity(b2Vec2{ body_c.base_speed * nrm_dir.x, body_c.base_speed * nrm_dir.y });
  }
}

} // namespace game2d