#include "spawner_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_death_throes/death_throes_components.hpp"
#include "modules/system_items_drop_on_death/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/ui_scene_survive_timer/ui_survive_timer_components.hpp"
#include "spawner_components.hpp"
#include "spawner_helpers.hpp"

#include <SDL2/SDL_log.h>

#include <unordered_map>

namespace game2d {

entt::entity
spawn_enemy(entt::registry& r, std::string key, float hp)
{
  // TODO: could have an "aggro meter" per player?
  auto target_e = get_random_player_target(r);
  if (target_e == entt::null)
    return entt::null;

  auto e = spawn(r, key);
  r.emplace<EnemyComponent>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::enemy });
  // r.emplace<SpriteOutline>(e);

  // get a random position around target player?
  // TODO: should be a larger zone considering all players?
  const auto& target_t = r.get<TransformComponent>(target_e);
  const auto rnd_pos = rnd_position_around_point(r, { target_t.position.x, target_t.position.y });
  give_life(r, e, rnd_pos, { 32, 32 });

  //
  // per-enemy tweaks
  //

  // pufferfish
  if (key == "actor_enemy_exploder") {
    r.emplace<DeathThroesComponent>(e);
    r.get<ApplyForceToDynamicTarget>(e).speed = 2.0f;
  }

  // horseshoe crab
  if (key == "actor_enemy_melee_1") {
    r.get<ApplyForceToDynamicTarget>(e).speed = 2.0f;
    r.emplace<RotateToVelocityComponent>(e);
    r.emplace<SetTransformRotationBasedOnPhysicsBody>(e);
  }

  // hermit crab
  if (key == "actor_enemy_melee_2") {
    // note: anything with ARC_ANGLE wants an ActorSpeedComponent
    r.emplace<ActorSpeedComponent>(e, 0.015f);
  }

  // red claw crab
  if (key == "actor_enemy_melee_3") {
    // note: anything with ARC_ANGLE wants an ActorSpeedComponent
    r.emplace<ActorSpeedComponent>(e, 0.015f);
  }

  // archerfish
  if (key == "actor_enemy_projectile") {
    r.get<ApplyForceToDynamicTarget>(e).distance_to_reduce_thrust = 6.0f;
  }

  // jellyfish
  if (key == "actor_enemy_swarmlord") {
    r.get<ApplyForceToDynamicTarget>(e).distance_to_reduce_thrust = 9.0f;
    r.emplace<CooldownComponent>(e);
    r.emplace<SwarmLordComponent>(e);
  }

  // sea urchin actor_enemy_grower
  // sea horse actor_enemy_charger
  // oyster = actor_destructable

  auto fixture_e = get_fixture_by_tag(r, e, "fixture_core");
  r.emplace<EnemyComponent>(fixture_e); // duplicate enemy component on fixture?
  r.emplace<HealthComponent>(fixture_e, hp, hp);
  // r.emplace<DefenceComponent>(fixture_e);

  // move at player, this gotta be changed for more interesting types
  r.emplace<DynamicTargetComponent>(e, target_e);
  r.emplace<PhysicsDynamicTarget>(e, target_e);

  auto& callbacks_c = r.get<OnDeathCallbacks>(e);
  auto drop_xp_callback = [](entt::registry& r, const entt::entity e) {
    // SDL_Log("Calling drop_xp_on_death_callback()");
    drop_xp_on_death_callback(r, e);
  };
  callbacks_c.callbacks.push_back(drop_xp_callback);

  return e;
};

void
update_spawner_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, survive_e, survive_c);

  // Get info from the survive timer
  const auto& survive_timer_c = r.get<CooldownComponent>(survive_e);
  const int seconds_from_start = survive_timer_c.time_max - (int)survive_timer_c.time;

  auto enemy_to_amount = get_live_enemies_map(r);

  const auto& view = r.view<CooldownComponent, const EnemySpawnData>();
  for (const auto& [spawner_e, cooldown_c, spawn_data] : view.each()) {
    if (cooldown_c.time > 0.0f)
      continue;

    // Get mob's spawner data
    const auto wave_opt = get_wave_from_time(spawn_data, seconds_from_start);
    if (!wave_opt.has_value())
      continue;
    const auto wave = wave_opt.value();

    // configs
    const auto max_allowed = wave.max;
    const auto number_per_spawn = wave.num_per_spawn;
    const auto hp = wave.hp;
    const auto cooldown = wave.spawn_cooldown;
    const auto key = spawn_data.key;

    // live data
    const int enemies = enemy_to_amount[key];

    bool allowed_to_spawn = true;
    allowed_to_spawn &= cooldown_c.time <= 0.0f; // not on cooldown
    allowed_to_spawn &= enemies < max_allowed;
    allowed_to_spawn &= (enemies + number_per_spawn) <= max_allowed;
    if (!allowed_to_spawn)
      continue;

    // spawn the thing
    for (int i = 0; i < number_per_spawn; i++)
      spawn_enemy(r, key, hp);

    // once spawned, put this mob's spawner on cooldown
    cooldown_c.time_max = cooldown.value();
    reset_cooldown(cooldown_c);

    // SDL_Log("spawning %i, cooldown: %i", number_per_spawn, cooldown);
  }
}

} // namespace game2d