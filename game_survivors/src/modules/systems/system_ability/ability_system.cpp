#include "pch.hpp"

#include "ability_components.hpp"
#include "ability_system.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_islander/islander_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/systems/system_particles/components.hpp"

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
    valid_target |= r.all_of<EnemyComponent>(e);
    valid_target |= r.all_of<PlayerBoatComponent>(e);
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
      const auto thing_body_id = r.get<PhysicsBodyComponent>(par_e).bodyId;
      const auto mass = b2Body_GetMass(thing_body_id);
      const auto impuse = mass * impulse_amount;
      b2Body_ApplyLinearImpulseToCenter(thing_body_id, impuse * b2Vec2{ nrm_dir.x, nrm_dir.y }, true);

      break; // if you collide with a valid fixture, apply force once.
    }
  }
};

void
speedboost_ability(entt::registry& r, entt::entity e, const InputComponent& input_c, const PhysicsBodyComponent& body_c)
{
  b2Body_ApplyForceToCenter(body_c.bodyId, { 5000.0f * input_c.lx, 5000.0f * input_c.ly }, true);
};

void
update_ability_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto view = r.view<const PlayerComponent, const InputComponent, const PhysicsBodyComponent, AbilityComponent>(
    entt::exclude<DroppedAnchorComponent>);
  for (const auto& [e, player_c, input_c, body_c, ability_c] : view.each()) {

    if (ability_c.ability_1_cooldown_left > 0.0f)
      ability_c.ability_1_cooldown_left -= dt;
    if (ability_c.ability_2_cooldown_left > 0.0f)
      ability_c.ability_2_cooldown_left -= dt;

    const bool allowed_to_use_ability_1 = ability_c.ability_1_cooldown_left <= 0.0f;
    const bool allowed_to_use_ability_2 = ability_c.ability_2_cooldown_left <= 0.0f;

    if (allowed_to_use_ability_1) {
      if (has(input_c.ability1, ActionStateEnum::DOWN)) {
        speedboost_ability(r, e, input_c, body_c);
        ability_c.ability_1_in_progress = true;
      }
      if (has(input_c.ability1, ActionStateEnum::HELD)) {
      }
      if (has(input_c.ability1, ActionStateEnum::RELEASE)) {
        ability_c.ability_1_in_progress = false;
        ability_c.ability_1_cooldown_left = ability_c.ability_1_cooldown;
      }
    }

    if (allowed_to_use_ability_2) {
      if (has(input_c.ability2, ActionStateEnum::DOWN)) {

        RequestToSpawnParticles request;
        request.key = "vfx_boop";
        request.position = get_position(r, e);
        create_empty<RequestToSpawnParticles>(r, request);

        boop_ability(r, e);
        ability_c.ability_2_in_progress = true;
      }
      if (has(input_c.ability2, ActionStateEnum::HELD)) {
      }
      if (has(input_c.ability2, ActionStateEnum::RELEASE)) {
        ability_c.ability_2_in_progress = false;
        ability_c.ability_2_cooldown_left = ability_c.ability_2_cooldown;
      }
    }
  }
}

} // namespace game2d