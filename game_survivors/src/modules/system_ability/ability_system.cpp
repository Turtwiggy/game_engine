#include "pch.hpp"

#include "ability_components.hpp"
#include "ability_system.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/actor_player_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"

namespace game2d {

void
boop_ability(entt::registry& r, entt::entity e)
{
  const auto pos = get_position(r, e);
  const b2Vec2 center_m = pixels_to_meters(pos);
  const float distance_m = 3.0f;
  const float impulse_amount = 8.0f;

  const std::function<bool(entt::registry&, entt::entity)> filter_criteria = [](entt::registry& r, entt::entity e) -> bool {
    bool valid_target = false;
    valid_target |= r.try_get<EnemyComponent>(e) != nullptr;
    valid_target |= r.try_get<PlayerComponent>(e) != nullptr;
    return valid_target;
  };
  const auto things_with_health = get_all_in_area_filtered(r, center_m, distance_m, filter_criteria);
  SDL_Log("Ability1: knocking back %zu things", things_with_health.size());

  // knock em all back!
  for (const auto& [par_e, coll_fixtures] : things_with_health) {

    for (const auto& fixture_coll_result : coll_fixtures) {
      const auto fixture_e = fixture_coll_result.fixture_e;
      const bool has_hp = r.try_get<HealthComponent>(fixture_e);
      if (!has_hp)
        continue; // shield or xp zone or something without health

      const auto raw_dir = get_position(r, par_e) - pos;
      const auto nrm_dir = engine::normalize_safe(raw_dir);
      const auto& thing_body = r.get<PhysicsBodyComponent>(par_e).body;
      const auto mass = thing_body->GetMass();
      const auto impuse = mass * impulse_amount;
      thing_body->ApplyLinearImpulseToCenter(impuse * b2Vec2{ nrm_dir.x, nrm_dir.y }, true);
      break; // if you collide with a valid fixture, apply force once.
    }
  }
};

void
anchor_down(entt::registry& r, entt::entity e)
{
  // signal state
  r.emplace_or_replace<LockedInSpotComponent>(e);
}

void
anchor_held(entt::registry& r, entt::entity e, const InputComponent& input_c, const PhysicsBodyComponent& body_c)
{
  // stop movement.
  auto& speed_c = r.get<ActorSpeedComponent>(e).current_speed = 0.0f;

  // let player rotate freely
  const float angle = engine::dir_to_angle_radians({ input_c.lx, input_c.ly });
  body_c.body->SetTransform(body_c.body->GetPosition(), angle);
}

void
anchor_release(entt::registry& r, entt::entity e, const InputComponent& input_c, const PhysicsBodyComponent& body_c)
{
  // reset speed
  auto& speed_c = r.get<ActorSpeedComponent>(e);
  speed_c.current_speed = speed_c.base_speed;

  // remove lock
  if (auto* locked_c = r.try_get<LockedInSpotComponent>(e))
    r.remove<LockedInSpotComponent>(e);

  // Give a speed boost? tokyo drifffftttttt
  const float meters_per_second = 5;
  body_c.body->SetLinearVelocity(meters_per_second * b2Vec2{ input_c.lx, input_c.ly });
};

void
update_ability_system(entt::registry& r, const float dt)
{
  return; // disabled

  for (const auto& [e, player_c, input_c, body_c, ability_c] :
       r.view<const PlayerComponent, const InputComponent, const PhysicsBodyComponent, AbilityComponent>().each()) {

    if (ability_c.ability_1_cooldown_left > 0.0f)
      ability_c.ability_1_cooldown_left -= dt;
    if (ability_c.ability_2_cooldown_left > 0.0f)
      ability_c.ability_2_cooldown_left -= dt;

    const bool allowed_to_use_ability_1 = ability_c.ability_1_cooldown_left <= 0.0f;
    const bool allowed_to_use_ability_2 = ability_c.ability_2_cooldown_left <= 0.0f;

    if (allowed_to_use_ability_1) {
      if (has_action(input_c.ability1, ActionStateEnum::DOWN)) {
        anchor_down(r, e);
        ability_c.ability_1_in_progress = true;
      }
      if (has_action(input_c.ability1, ActionStateEnum::HELD)) {
        anchor_held(r, e, input_c, body_c);
      }
      if (has_action(input_c.ability1, ActionStateEnum::RELEASE)) {
        anchor_release(r, e, input_c, body_c);
        ability_c.ability_1_in_progress = false;
        ability_c.ability_1_cooldown_left = ability_c.ability_1_cooldown;
      }
    }

    if (allowed_to_use_ability_2) {
      if (has_action(input_c.ability2, ActionStateEnum::DOWN)) {
        boop_ability(r, e);
        ability_c.ability_2_in_progress = true;
      }
      if (has_action(input_c.ability2, ActionStateEnum::HELD)) {
      }
      if (has_action(input_c.ability2, ActionStateEnum::RELEASE)) {
        ability_c.ability_2_in_progress = false;
        ability_c.ability_2_cooldown_left = ability_c.ability_2_cooldown;
      }
    }
  }
}

} // namespace game2d