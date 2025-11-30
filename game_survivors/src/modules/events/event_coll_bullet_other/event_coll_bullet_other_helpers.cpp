#include "pch.hpp"

#include "event_coll_bullet_other_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_components.hpp"
#include "modules/combat/combat_weapon_type_projectile/combat_weapon_type_projectile_components.hpp"
#include "modules/events/event_coll/event_coll_components.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/ui/ui_debug_effects/effects_helpers.hpp"

namespace game2d {

void
handle_bullet_other_coll(entt::registry& r, const OnCollisionEnter& coll_evt)
{
  const auto [other_fixture_e, bullet_fixture_e] = coll<PhysicsFixtureComponent, BulletComponent>(r, coll_evt.a, coll_evt.b);
  if (other_fixture_e == entt::null || bullet_fixture_e == entt::null)
    return;

  // Get the top-level parent, where most of the components will be
  entt::entity other_e_parent = other_fixture_e;
  if (auto* has_parent_c = r.try_get<HasParentComponent>(other_fixture_e))
    other_e_parent = has_parent_c->parent;

  entt::entity bullet_e_parent = bullet_fixture_e;
  if (auto* has_parent_c = r.try_get<HasParentComponent>(bullet_fixture_e))
    bullet_e_parent = has_parent_c->parent;

#if defined(_DEBUG)
// const auto& parent_tag = r.get<TagComponent>(other_e_parent).tag;
#endif
  const auto& fixture_tag = r.get<TagComponent>(other_fixture_e).tag;

  const auto& bullet_team_c = r.get<TeamComponent>(bullet_e_parent);
  const auto* other_team_c = r.try_get<TeamComponent>(other_e_parent);
  if (!other_team_c)
    return; // e.g. the flamethrower flame has a fixture but no team
  if (bullet_team_c.team == other_team_c->team)
    return; // dont damage same team

  // Make sure fixture is valid
  {
    const auto valid_fixture_tags = std::vector<std::string>{ "fixture_core", "fixture_player", "fixture_shield" };
    const auto it = std::find(valid_fixture_tags.begin(), valid_fixture_tags.end(), fixture_tag);
    if (it == valid_fixture_tags.end())
      return;
  }

  // Here, a bullet has collided with something on a different team.
  // Check that the bullet has not collided with this entity before.
  auto& coll = r.get_or_emplace<CollInfo>(bullet_e_parent).other;
  auto it = std::find(coll.begin(), coll.end(), other_e_parent);
  bool bullet_already_coll_with_other = it != coll.end();
  if (bullet_already_coll_with_other)
    return;
  coll.emplace(other_e_parent);

  auto& evts_c = SINGLE_Events::instance;

  // note: these values have already been
  // modified with upgrades at the point they were created
  const auto& bullet_traits_c = r.get<WeaponBehaviourComponent>(bullet_e_parent);
  const auto& bullet_damage_c = r.get<BulletDamage>(bullet_e_parent);
  const auto& bullet_knockback_c = r.get<BulletKnockback>(bullet_e_parent);
  auto& bullet_pierce_c = r.get<BulletPierce>(bullet_e_parent);
  const auto& bullet_damage_type_c = r.get<WeaponDamageTypeComponent>(bullet_e_parent);

  // Send a damage event from the bullet to the other entity
  {
    DamageEvent evt;
    evt.from = bullet_e_parent;
    evt.to_parent = other_e_parent;
    evt.to_fixture = other_fixture_e;
    evt.type = bullet_damage_type_c.type;
    evt.amount = bullet_damage_c.damage;
    evts_c.dispatcher->trigger(evt);
    evts_c.dispatcher->update();
  }

  auto* hp_c = r.try_get<HealthComponent>(other_fixture_e);
  if (hp_c) {
    //
    // give bullets "pierce" as the num enemies you can hit
    //
    {
      bullet_pierce_c.pierced++;
      if (bullet_pierce_c.pierced >= bullet_pierce_c.pierce) {
        // maximum number of enemies pierced with this bullet
        auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
        dead.dead.push_back(bullet_e_parent);
      }
    }
  }

  const auto reverse_velocity = [&r, bullet_e_parent]() {
    auto& bullet_body_c = r.get<PhysicsBodyComponent>(bullet_e_parent);
    b2Body_SetLinearVelocity(bullet_body_c.bodyId, -1.0 * b2Body_GetLinearVelocity(bullet_body_c.bodyId));
  };

  // const auto is_scenery = r.try_get<RockComponent>(other_e_parent) != nullptr;
  // else if (is_scenery)
  //   reverse_velocity();

  // Reverse yo velocity
  // Note: this should work as bullets only collide once with enemies.
  auto* bullet_bounce_c = r.try_get<BulletBounce>(bullet_e_parent);
  if (bullet_bounce_c && bullet_bounce_c->bounces_left > 0) {
    reverse_velocity();
    bullet_bounce_c->bounces_left--;
  } else if (fixture_tag == "fixture_shield") {
    reverse_velocity();

    if (!hp_c) {
      // create a piece of worldspace text saying "blocked" as no damage was given.
      // (unless a health component is ever added to the shield, which it might be)
      // create_popup(r, get_position(r, bullet_e_parent), "blocked!");

      // create a block vfx
      // static engine::RandomState rnd(0);
      // const auto random_idx = engine::rand_det_s(rnd.rng, 0, 2); // 0 or 1
      // SDL_Log("Block random idx: %i", random_idx);
      const auto fx_str = std::format("BLOCK_FX_{}", 0);
      spawn_fx(r, fx_str, get_position(r, bullet_e_parent), { 32, 32 });

      r.emplace_or_replace<RequestHitScaleComponent>(other_fixture_e, RequestHitScaleComponent{ .scale_up_pixels = 10.0f });
    }
  }

  // knockback applies to "core" and "shield"
  if (fixture_tag == "fixture_core" || fixture_tag == "fixture_shield") {
    // Knockback the enemy
    if (other_team_c->team == AvailableTeams::enemy) {
      const auto& enemy_body_c = r.get<PhysicsBodyComponent>(other_e_parent);
      const auto raw_dir = get_position(r, other_e_parent) - get_position(r, bullet_e_parent);
      const auto nrm_dir = engine::normalize_safe(raw_dir);

      const float knockback_force = bullet_knockback_c.knockback_force;

      // clamp knockback force.
      const float clamped_knockback_force = glm::min(knockback_force, 2.0f);
      b2Body_SetLinearVelocity(enemy_body_c.bodyId, clamped_knockback_force * b2Vec2{ nrm_dir.x, nrm_dir.y });

      // spawn impact vfx at the bullet position
      spawn_fx(r, "S6_EXPLODE_FX_14", get_position(r, bullet_e_parent), { 16, 16 });
    }
  }
}

} // namespace game2d