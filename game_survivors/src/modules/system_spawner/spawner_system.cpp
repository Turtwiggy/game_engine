#include "spawner_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_items_drop_on_death/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/ui_survive_timer/ui_survive_timer_components.hpp"
#include "spawner_components.hpp"
#include "spawner_helpers.hpp"

#include <SDL2/SDL_log.h>
#include <unordered_map>

namespace game2d {

entt::entity
spawn_enemy(entt::registry& r, std::string key, int hp)
{
  // TODO: could have an "aggro meter" per player?
  auto target_e = get_random_player_target(r);
  if (target_e == entt::null)
    return entt::null;

  auto e = spawn(r, key);
  r.emplace<EnemyComponent>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::enemy });
  r.emplace_or_replace<HealthComponent>(e, hp, hp);
  // r.emplace<SpriteOutline>(e);

  // move at player, this gotta be changed for more interesting types
  r.emplace<DynamicTargetComponent>(e, target_e);
  r.emplace<PhysicsDynamicTarget>(e, target_e);

  auto& callbacks_c = r.get<OnDeathCallbacks>(e);
  auto drop_xp_callback = [](entt::registry& r, const entt::entity e) {
    SDL_Log("Calling drop_xp_on_death_callback()");
    drop_xp_on_death_callback(r, e);
  };
  callbacks_c.callbacks.push_back(drop_xp_callback);

  // get a random position around target player?
  // TODO: : it should be a larger zone considering all players
  const auto& target_t = r.get<TransformComponent>(target_e);
  const auto rnd_pos = rnd_position_around_point(r, { target_t.position.x, target_t.position.y });
  give_life(r, e, rnd_pos, { 16, 16 });

  return e;
};

std::unordered_map<std::string, int>
get_live_enemies_map(entt::registry& r)
{
  // How many of each enemies do we currently have?
  const auto& enemies_view = r.view<EnemyComponent, Item>();
  std::unordered_map<std::string, int> enemy_to_amount;
  for (const auto& [e, enemy_c, item_c] : enemies_view.each())
    enemy_to_amount[item_c.name] += 1;
  return enemy_to_amount;
}

void
update_spawner_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, survive_e, survive_c);
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c);

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
    const int max_allowed = wave.max_allowed;
    const int number_per_spawn = wave.number_per_spawn;
    const int hp = wave.hp;
    const int cooldown = wave.spawn_cooldown;
    const std::string key = spawn_data.enemy_key;

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
    cooldown_c.time_max = cooldown;
    reset_cooldown(cooldown_c);

    // SDL_Log("spawning %i, cooldown: %i", number_per_spawn, cooldown);
  }
}

} // namespace game2d