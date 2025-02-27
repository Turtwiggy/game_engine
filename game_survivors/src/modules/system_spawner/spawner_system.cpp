#include "spawner_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_enemy_grower/enemy_grower_components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/core_colour/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/core_renderer/components.hpp"
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
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  // TODO: could have an "aggro meter" per player?
  auto target_e = get_random_player_target(r);
  if (target_e == entt::null)
    return entt::null;

  // get a random position around target player?
  // TODO: should be a larger zone considering all players?
  const auto& target_t = r.get<TransformComponent>(target_e);

  const auto target_pos = glm::vec2{ target_t.position.x, target_t.position.y };
  const float screen_max = glm::max(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const auto rnd_pos_around_player = rnd_position_around_point(r, target_pos, screen_max, screen_max);

  auto e = spawn(r, key);
  r.emplace<EnemyComponent>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::enemy });
  // r.emplace<SpriteOutline>(e);

  auto enemy_size = glm::vec2{ 32, 32 };
  if (key == "actor_enemy_swarmlord_minion")
    enemy_size = { 16, 16 };
  if (key == "actor_enemy_grower")
    enemy_size = { 0, 0 };

  give_life(r, e, rnd_pos_around_player, enemy_size);

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

    // remove your single sprite, and create 2 sprites.
    // one for your legs, one for your house
    r.remove<TransformComponent>(e);
    r.remove<SpriteComponent>(e);
    const auto& pb_c = r.get<PhysicsBodyComponent>(e);
    for (int i = 0; const auto& fixture_e : pb_c.fixtures) {
      r.emplace<TransformComponent>(fixture_e);
      r.emplace<SpriteComponent>(fixture_e);
      auto col = engine::SRGBColour{ 1.0f, 1.0f, 1.0f, 1.0f };
      r.emplace<DefaultColour>(fixture_e, col);
      set_colour(r, fixture_e, col);

      // const auto& item_key_c = r.get<ItemKey>(e);
      // const auto item_c = find_item(r, item_key_c.key);
      // const auto fixture_size = item_c.phys_fixtures.value()[i].size[0];
      // const auto size = glm::ivec2{ fixture_size.x, fixture_size.y };
      r.emplace<DefaultSizeComponent>(fixture_e, enemy_size);
      set_size(r, fixture_e, enemy_size);

      if (i == 0) // fixture_core
        set_sprite(r, fixture_e, "HERMIT_CRAB_BOTTOM");
      if (i == 1) // shield
        set_sprite(r, fixture_e, "HERMIT_CRAB_TOP");

      i++;
    }
  }

  // red claw crab
  if (key == "actor_enemy_melee_3") {
    // note: anything with ARC_ANGLE wants an ActorSpeedComponent
    r.emplace<ActorSpeedComponent>(e, 0.015f);

    // remove your single sprite, and create 2 sprites.
    // one for your left half, one for your right half
    r.remove<TransformComponent>(e);
    r.remove<SpriteComponent>(e);
    const auto& pb_c = r.get<PhysicsBodyComponent>(e);
    for (int i = 0; const auto& fixture_e : pb_c.fixtures) {
      r.emplace<TransformComponent>(fixture_e);
      r.emplace<SpriteComponent>(fixture_e);
      auto col = engine::SRGBColour{ 1.0f, 1.0f, 1.0f, 1.0f };
      r.emplace<DefaultColour>(fixture_e, col);
      set_colour(r, fixture_e, col);

      r.emplace<DefaultSizeComponent>(fixture_e, enemy_size);
      set_size(r, fixture_e, enemy_size);

      if (i == 0) // fixture_core
        set_sprite(r, fixture_e, "REDCLAW_CRAB_LEFT");
      if (i == 1) // shield
        set_sprite(r, fixture_e, "REDCLAW_CRAB_RIGHT");

      i++;
    }
  }

  // archerfish
  if (key == "actor_enemy_projectile") {
    r.get<ApplyForceToDynamicTarget>(e).distance_to_reduce_thrust = 6.0f;
  }

  // jellyfish
  if (key == "actor_enemy_swarmlord") {
    r.get<ApplyForceToDynamicTarget>(e).distance_to_reduce_thrust = 9.0f;
    r.emplace<CooldownComponent>(e,
                                 CooldownComponent{
                                   .time_max = 7.0f,
                                 });
    r.emplace<SwarmLordComponent>(e);
  }

  // sea urchin
  if (key == "actor_enemy_grower") {
    r.emplace<GrowerComponent>(e);

    const auto rnd_pos_inside_map = rnd_position_in_map_but_not_inside_players(r);
    set_position(r, e, rnd_pos_inside_map);
  }

  // sea horse
  if (key == "actor_enemy_charger") {
  }

  if (key == "actor_destructable") {
    //
  }

  auto fixture_e = get_fixture_by_tag(r, e, "fixture_core");
  r.emplace<EnemyComponent>(fixture_e); // duplicate enemy component on fixture?
  r.emplace<HealthComponent>(fixture_e, hp, hp);

  if (key == "actor_enemy_grower") {
    // A grower's health is it's size, not a healthcomponent
    r.remove<HealthComponent>(fixture_e);
  }

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