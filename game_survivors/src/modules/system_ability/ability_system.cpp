#include "pch.hpp"

#include "ability_components.hpp"
#include "ability_system.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"

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
  for (const auto& [thing_distance, thing_e] : things_with_health) {
    const auto raw_dir = get_position(r, thing_e) - pos;
    const auto nrm_dir = engine::normalize_safe(raw_dir);
    const auto& thing_body = r.get<PhysicsBodyComponent>(thing_e).body;
    const auto mass = thing_body->GetMass();
    const auto impuse = mass * impulse_amount;
    thing_body->ApplyLinearImpulseToCenter(impuse * b2Vec2{ nrm_dir.x, nrm_dir.y }, true);
  }
};

void
anchor_down(entt::registry& r, entt::entity e)
{
  // signal state
  r.emplace<LockedInSpotComponent>(e);
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
  r.remove<LockedInSpotComponent>(e);

  // Give a speed boost? tokyo drifffftttttt
  const auto mass = body_c.body->GetMass();
  body_c.body->SetLinearVelocity(100.0f * mass * speed_c.current_speed * b2Vec2{ input_c.lx, input_c.ly });
}

void
update_ability_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  const auto nz_handles = non_zero_handles(steam_gs_c.handles);

  for (const auto& [e, player_c, input_c, body_c, ability_c] :
       r.view<const PlayerComponent, const InputComponent, const PhysicsBodyComponent, AbilityComponent>().each()) {

    auto* controller_c = r.try_get<SteamControllerComponent>(e);
    if (!controller_c)
      continue; // should check for keyboard

    const auto handle = controller_c->handle;
    if (handle == 0)
      continue;

    if (ability_c.ability_1_cooldown_left > 0.0f)
      ability_c.ability_1_cooldown_left -= dt;
    if (ability_c.ability_2_cooldown_left > 0.0f)
      ability_c.ability_2_cooldown_left -= dt;

    const bool allowed_to_use_ability_1 = ability_c.ability_1_cooldown_left <= 0.0f;
    const bool allowed_to_use_ability_2 = ability_c.ability_2_cooldown_left <= 0.0f;

    if (allowed_to_use_ability_1 && controller_button_down(steam_c, handle, DA::Game_Ability1)) {
      anchor_down(r, e);
      ability_c.ability_1_in_progress = true;
    }
    if (allowed_to_use_ability_1 && controller_button_held(steam_c, handle, DA::Game_Ability1)) {
      anchor_held(r, e, input_c, body_c);
    }
    if (allowed_to_use_ability_1 && controller_button_release(steam_c, handle, DA::Game_Ability1)) {
      anchor_release(r, e, input_c, body_c);
      ability_c.ability_1_in_progress = false;
      ability_c.ability_1_cooldown_left = ability_c.ability_1_cooldown;
    }

    if (allowed_to_use_ability_2 && controller_button_down(steam_c, handle, DA::Game_Ability2)) {
      boop_ability(r, e);
      ability_c.ability_2_in_progress = true;
    }
    if (allowed_to_use_ability_2 && controller_button_held(steam_c, handle, DA::Game_Ability2)) {
    }
    if (allowed_to_use_ability_2 && controller_button_release(steam_c, handle, DA::Game_Ability2)) {
      ability_c.ability_2_in_progress = false;
      ability_c.ability_2_cooldown_left = ability_c.ability_2_cooldown;
    }
  }
}

} // namespace game2d