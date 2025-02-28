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
#include "modules/actor_enemy_treasure/enemy_treasure_components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/core_animations/wiggle/components.hpp"
#include "modules/core_colour/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/core_renderer/helpers.hpp"
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

  auto& callbacks_c = r.get<OnDeathCallbacks>(e);
  auto drop_xp_callback = [](entt::registry& r, const entt::entity e) { drop_xp_on_death_callback(r, e); };
  callbacks_c.callbacks.push_back(drop_xp_callback);

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

  // oyster
  if (key == "actor_destructable") {
    r.emplace<TreasureEnemyComponent>(e);

    // Dont drop xp. (drop something else)
    callbacks_c.callbacks.clear();

    const auto rnd_pos_inside_map = rnd_position_in_map_but_not_inside_players(r);
    set_position(r, e, rnd_pos_inside_map);

    // create a gold halo
    auto halo_e = create_transform(r, "GOLD_OUTLINE");
    r.emplace<SpriteComponent>(halo_e);
    set_sprite(r, halo_e, "GOLD_OUTLINE");
    set_position(r, halo_e, rnd_pos_inside_map);
    set_size(r, halo_e, 1.0f * enemy_size);
    r.emplace<HasParentComponent>(halo_e, e);
    r.emplace<WiggleUpAndDown>(halo_e,
                               WiggleUpAndDown{
                                 .base_position = rnd_pos_inside_map,
                                 .amplitude = 2.0f,
                                 .frequency = 4.0f,
                               });
    set_z_index(r, halo_e, ZLayer::VFX);
    auto& enemy_children_c = r.get_or_emplace<HasChildrenComponent>(e);
    enemy_children_c.children.push_back(halo_e);
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

  return e;
};

void
update_spawner_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, survive_e, survive_c);
  GET_FIRST_OR_RETURN(SINGLE_SpawnerLiveData, r, live_spawn_data_e, live_spawn_data_c);
  GET_FIRST_OR_RETURN(SINGLE_Spawners, r, disk_spawn_data_e, disk_spawn_data_c);

  // Get info from the survive timer
  const auto& survive_timer_c = r.get<CooldownComponent>(survive_e);
  const int seconds_from_start = survive_timer_c.time_max - (int)survive_timer_c.time;

  // How many of each enemies do we currently have?
  const auto& enemies_view = r.view<const EnemyComponent, const ItemKey>();
  std::unordered_map<std::string, int> enemy_to_amount;
  for (const auto& [e, enemy_c, item_c] : enemies_view.each())
    enemy_to_amount[item_c.key] += 1;

  for (const auto& [spawner_e, cooldown_c, spawn_data] : r.view<CooldownComponent, const EnemySpawnData>().each()) {
    if (cooldown_c.time > 0.0f)
      continue;

    // Get mob's spawner data
    const auto wave_opt = get_wave_index_from_time(spawn_data, seconds_from_start);
    if (!wave_opt.has_value())
      continue;

    const auto wave_key = WaveKey{
      .on_disk_spawns_index = spawn_data.on_disk_index,
      .on_disk_waves_index = wave_opt.value(),
    };
    const auto on_disk_wave = disk_spawn_data_c.spawns[wave_key.on_disk_spawns_index].waves[wave_key.on_disk_waves_index];

    // configs
    const auto max_to_spawn_this_wave_opt = on_disk_wave.num_per_wave;
    const auto max_allowed = on_disk_wave.max;
    const auto number_per_spawn = on_disk_wave.num_per_spawn;
    const auto hp = on_disk_wave.hp;
    const auto cooldown = on_disk_wave.spawn_cooldown;
    const auto enemy_key = spawn_data.key;

    // live data
    const int enemies = enemy_to_amount[enemy_key];
    const auto has_wave_data = live_spawn_data_c.data.contains(wave_key);
    if (!has_wave_data)
      live_spawn_data_c.data[wave_key] = {};

    // spawn conditions
    bool allowed_to_spawn = true;
    allowed_to_spawn &= cooldown_c.time <= 0.0f; // not on cooldown
    allowed_to_spawn &= enemies < max_allowed;
    allowed_to_spawn &= (enemies + number_per_spawn) <= max_allowed;

    // limit: if you only want to spawn X enemies this wave instead of continuous
    if (max_to_spawn_this_wave_opt.has_value()) {
      const int enemies_spawned = live_spawn_data_c.data[wave_key].spawned;
      allowed_to_spawn &= (enemies_spawned + number_per_spawn) <= max_to_spawn_this_wave_opt.value();
    }

    if (!allowed_to_spawn)
      continue;

    // spawn the thing
    for (int i = 0; i < number_per_spawn; i++) {
      spawn_enemy(r, enemy_key, hp);
      live_spawn_data_c.data[wave_key].spawned++;
    }

    // once spawned, put this mob's spawner on cooldown
    cooldown_c.time_max = cooldown.value();
    reset_cooldown(cooldown_c);

    // SDL_Log("spawning %i, cooldown: %i", number_per_spawn, cooldown);
  }
}

} // namespace game2d