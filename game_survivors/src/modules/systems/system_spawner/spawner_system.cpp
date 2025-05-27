#include "pch.hpp"

#include "spawner_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_enemy_grower/enemy_grower_components.hpp"
#include "modules/actors/actor_enemy_treasure/enemy_treasure_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_snake/snake_components.hpp"
#include "modules/actors/actor_snake/snake_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_components.hpp"
#include "modules/core/animations/wiggle/components.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/effects_outline/outline_components.hpp"
#include "modules/systems/system_combo_unlock/combo_unlock_components.hpp"
#include "modules/systems/system_combo_unlock/combo_unlock_helpers.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_cooldown/helpers.hpp"
#include "modules/systems/system_death_throes/death_throes_components.hpp"
#include "modules/systems/system_items_drop_on_death/helpers.hpp"
#include "modules/systems/system_move_to_target_via_lerp/components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_upgrade_dodge/upgrade_dodge_components.hpp"
#include "modules/systems/system_upgrade_hp_regen/upgrade_hp_regen_components.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_components.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_helpers.hpp"
#include "modules/ui/ui_scene_survive_timer/ui_survive_timer_components.hpp"
#include "spawner_components.hpp"
#include "spawner_helpers.hpp"

namespace game2d {

entt::entity
spawn_enemy(entt::registry& r, std::string key, float hp)
{
  const auto& ri = SINGLE_RendererInfo::instance;

  // hack: multiply hp by number of players.
  const auto num_players = r.view<PlayerComponent>().size();
  hp *= glm::max((float)num_players, 1.0f);

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

  // boss, so it's a bit different
  if (key == "actor_snake")
    return create_snake(r);

  auto enemy_size = glm::vec2{ 32, 32 };
  if (key == "actor_enemy_swarmlord_minion")
    enemy_size = { 16, 16 };
  if (key == "actor_enemy_grower")
    enemy_size = { 0, 0 };

  // oyster
  entt::entity halo_e = entt::null;
  if (key == "actor_destructable") {
    halo_e = create_transform(r, "GOLD_OUTLINE");
    r.emplace<SpriteComponent>(halo_e);
    set_sprite(r, halo_e, "GOLD_OUTLINE");
    set_size(r, halo_e, 1.5f * enemy_size);
  }

  auto e = spawn(r, key);
  r.emplace<EnemyComponent>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::enemy });

  // Make it a variant.
  // outline it
  // 10x the HP, but it also drops a level up.
  static engine::RandomState variant_rng(0);
  const float variant_chance_percent_0_100 = 0.25f; // 0.25%
  const float variant_hp_multiplier = 10.0f;
  const bool is_variant = engine::rand_det_s(variant_rng.rng, 0, 100) < variant_chance_percent_0_100;
  if (is_variant && key != "actor_destructable") {
    r.emplace<SpriteOutline>(e);
    hp *= variant_hp_multiplier;
    auto& death_c = r.get<OnDeathCallbacks>(e);
    auto drop_xp_callback = [](entt::registry& r, const entt::entity e) { drop_levelup_xp_on_death_callback(r, e); };
    death_c.callbacks.push_back(drop_xp_callback);
  }

  // check global hp multipler
  {
    auto option = get_modifier_option(r, MODIFIER_OPTIONS::ENEMY_HEALTH);
    if (auto* o = dynamic_cast<Option_EnemyHealth*>(option.get())) {
      SDL_Log("Spawning enemy with global hp modifier: %f", o->multiplier);
      hp *= o->multiplier;
    }
  }

  give_life(r, e, rnd_pos_around_player, enemy_size);

  if (!is_variant) {
    auto& callbacks_c = r.get<OnDeathCallbacks>(e);
    auto drop_xp_callback = [](entt::registry& r, const entt::entity e) { drop_xp_on_death_callback(r, e); };
    callbacks_c.callbacks.push_back(drop_xp_callback);
  }

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
    for (int i = 0; const auto fixture_e : pb_c.fixtures) {

      const auto col = engine::SRGBColour{ 1.0f, 1.0f, 1.0f, 1.0f };
      r.emplace<TransformComponent>(fixture_e);
      r.emplace<SpriteComponent>(fixture_e);
      r.emplace<DefaultColour>(fixture_e, col);
      set_colour(r, fixture_e, col);
      set_position(r, fixture_e, rnd_pos_around_player);

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
      if (i == 1) // shield
        r.emplace<ShieldComponent>(fixture_e);

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
      set_position(r, fixture_e, rnd_pos_around_player);

      r.emplace<DefaultSizeComponent>(fixture_e, enemy_size);
      set_size(r, fixture_e, enemy_size);

      if (i == 0) // fixture_core
        set_sprite(r, fixture_e, "REDCLAW_CRAB_LEFT");
      if (i == 1) // shield
        set_sprite(r, fixture_e, "REDCLAW_CRAB_RIGHT");
      if (i == 1) // shield
        r.emplace<ShieldComponent>(fixture_e);

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
  if (key == "actor_enemy_swarmlord_minion") {
    auto& force_c = r.get<ApplyForceToDynamicTarget>(e);
    force_c.speed = 2.0f;
  }

  // sea urchin
  if (key == "actor_enemy_grower") {
    r.emplace<GrowerComponent>(e);

    ActorHealthRegenComponent regen_c;

    // TODO: probably move this to all enemies at some point

    // regen 0.1f hp/s
    StatModifierComponent stat_c;

    const auto mod_key = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_HEALTH_REGEN));
    const auto mod_val = 0.1f;
    stat_c.add(std::make_shared<StatFlatIncrease>(mod_val, mod_key));
    r.emplace<StatModifierComponent>(e, stat_c);
    r.emplace<ActorHealthRegenComponent>(e, regen_c); // parent not fixture
    r.emplace<ActorDodgeComponent>(e, 0.0f);

    const auto rnd_pos_inside_map = rnd_position_in_map_but_not_inside_players_or_islands(r);
    set_position(r, e, rnd_pos_inside_map);
  }

  // sea horse
  if (key == "actor_enemy_charger") {
  }

  auto fixture_e = get_fixture_by_tag(r, e, "fixture_core");
  r.emplace<EnemyComponent>(fixture_e); // duplicate enemy component on fixture?
  r.emplace<HealthComponent>(fixture_e, hp, hp);

  // oyster
  if (key == "actor_destructable") {
    r.emplace<TreasureEnemyComponent>(e);

    const auto rnd_pos_inside_map = rnd_position_in_map_but_not_inside_players_or_islands(r);
    set_position(r, e, rnd_pos_inside_map);
    set_position(r, halo_e, rnd_pos_inside_map);

    // Dont drop xp. (drop something else)
    auto& callbacks_c = r.get<OnDeathCallbacks>(e);
    callbacks_c.callbacks.clear();

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

    // make it in to a combo unlockable thing
    r.remove<HealthComponent>(fixture_e);
    r.emplace<ComboUnlockComponent>(e, generate_combo_component(r));
  }

  if (key == "actor_enemy_grower") {
    // start the grower as injured
    auto& hp_c = r.get<HealthComponent>(fixture_e);
    hp_c.hp = 1;
    hp_c.max_hp = hp;
  }

  // r.emplace<DefenceComponent>(fixture_e);

  // move at player, this gotta be changed for more interesting types
  r.emplace<DynamicTargetComponent>(e, target_e);
  r.emplace<PhysicsDynamicTarget>(e, target_e);

  return e;
};

void
update_wave_spawner(entt::registry& r, const std::unordered_map<std::string, int>& enemy_to_amount)
{
  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, survive_e, survive_c);
  GET_FIRST_OR_RETURN(SINGLE_OnDiskSpawners, r, disk_spawn_data_e, disk_spawn_data_c);
  GET_FIRST_OR_RETURN(SpawnerLiveData, r, live_spawn_data_e, live_spawn_data_c);

  const int seconds_from_start = survive_c.time_left_max - survive_c.time_left_cur;

  for (const auto& [spawner_e, cooldown_c, wave] : r.view<CooldownComponent, const EnemyWavesData>().each()) {
    if (cooldown_c.time > 0.0f)
      continue;

    // Filter wave by time.
    const bool in_lower_bound = seconds_from_start >= min_to_sec(wave.time.start);
    const bool in_upper_bound = seconds_from_start < min_to_sec(wave.time.stop);
    if (!in_lower_bound || !in_upper_bound)
      continue; // not this wave.

    //
    // All the enemies that this wave wants to be spawning
    //
    for (int idx = 0; const auto& wave_data : wave.enemies) {
      const auto enemy_key = wave_data.key;
      const auto data = wave_data.data;

      const WaveSpawnerWaveKey wave_key{
        .idx_in_wave_spawner = wave.on_disk_index,
        .idx_in_wave_spawner_enemy = idx++,
      };

      // live data
      int enemies = 0;
      if (enemy_to_amount.contains(enemy_key))
        enemies = enemy_to_amount.at(enemy_key);
      const auto has_wave_data = live_spawn_data_c.wavespawner_data.contains(wave_key);
      if (!has_wave_data)
        live_spawn_data_c.wavespawner_data[wave_key] = {};

      // check global enemy count multiplier
      float max = data.max;
      {
        auto option = get_modifier_option(r, MODIFIER_OPTIONS::ENEMY_COUNT);
        if (auto* o = dynamic_cast<Option_EnemyCount*>(option.get())) {
          // SDL_Log("Spawning enemy count modified by: %f", o->multiplier);
          max *= o->multiplier;
        }
      }

      // spawn conditions
      bool allowed_to_spawn = (enemies + data.num_per_spawn) <= max;

      // limit: if you only want to spawn X enemies this wave instead of continuous
      if (data.num_per_wave.has_value()) {
        const int enemies_spawned = live_spawn_data_c.wavespawner_data[wave_key].spawned;
        allowed_to_spawn &= (enemies_spawned + data.num_per_spawn) <= data.num_per_wave.value();
      }

      if (!allowed_to_spawn)
        continue;

      // spawn the thing
      for (int i = 0; i < data.num_per_spawn; i++) {
        spawn_enemy(r, enemy_key, data.hp);
        live_spawn_data_c.wavespawner_data[wave_key].spawned++;
      }

      //
    }

    // once spawned, put this mob's spawner on cooldown
    // cooldown_c.time_max = data.spawn_cooldown.value();
    cooldown_c.time_max = 2.0f; // time between spawner checks
    reset_cooldown(cooldown_c);

    //
  }
};

void
update_enemy_spawner(entt::registry& r, const std::unordered_map<std::string, int>& enemy_to_amount)
{
  GET_FIRST_OR_RETURN(SINGLE_OnDiskSpawners, r, disk_spawn_data_e, disk_spawn_data_c);
  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, survive_e, survive_c);
  GET_FIRST_OR_RETURN(SpawnerLiveData, r, live_spawn_data_e, live_spawn_data_c);

  const int seconds_from_start = survive_c.time_left_max - survive_c.time_left_cur;

  for (const auto& [spawner_e, cooldown_c, spawn_data] : r.view<CooldownComponent, const EnemySpawnsData>().each()) {
    if (cooldown_c.time > 0.0f)
      continue;

    // Get mob's spawner data
    const auto wave_opt = get_wave_index_from_time(spawn_data, seconds_from_start);
    if (!wave_opt.has_value())
      continue;

    const auto wave_key = EnemySpawnerWaveKey{
      .idx_in_enemy_spawner = spawn_data.on_disk_index,
      .idx_in_enemy_spawner_waves = wave_opt.value(),
    };

    const auto on_disk_wave =
      disk_spawn_data_c.enemy_spawner[wave_key.idx_in_enemy_spawner].waves[wave_key.idx_in_enemy_spawner_waves];

    // configs
    const auto enemy_key = spawn_data.key;
    const auto max_to_spawn_this_wave_opt = on_disk_wave.data.num_per_wave;
    const auto number_per_spawn = on_disk_wave.data.num_per_spawn;
    const auto hp = on_disk_wave.data.hp;
    // const auto cooldown = on_disk_wave.data.spawn_cooldown;
    float max = on_disk_wave.data.max;

    // live data
    int enemies = 0;
    if (enemy_to_amount.contains(enemy_key))
      enemies = enemy_to_amount.at(enemy_key);
    const auto has_wave_data = live_spawn_data_c.enemyspawner_data.contains(wave_key);
    if (!has_wave_data)
      live_spawn_data_c.enemyspawner_data[wave_key] = {};

    // check global enemy count multiplier
    {
      auto option = get_modifier_option(r, MODIFIER_OPTIONS::ENEMY_COUNT);
      if (auto* o = dynamic_cast<Option_EnemyCount*>(option.get())) {
        // SDL_Log("Spawning enemy count modified by: %f", o->multiplier);
        max *= o->multiplier;
      }
    }

    // spawn conditions
    bool allowed_to_spawn = (enemies + number_per_spawn) <= max;

    // limit: if you only want to spawn X enemies this wave instead of continuous
    if (max_to_spawn_this_wave_opt.has_value()) {
      const int enemies_spawned = live_spawn_data_c.enemyspawner_data[wave_key].spawned;
      allowed_to_spawn &= (enemies_spawned + number_per_spawn) <= max_to_spawn_this_wave_opt.value();
    }

    if (!allowed_to_spawn)
      continue;

    // spawn the thing
    for (int i = 0; i < number_per_spawn; i++) {
      spawn_enemy(r, enemy_key, hp);
      live_spawn_data_c.enemyspawner_data[wave_key].spawned++;
    }

    // once spawned, put this mob's spawner on cooldown
    // cooldown_c.time_max = cooldown.value();

    cooldown_c.time_max = 2.0f; // time between spawner checks
    reset_cooldown(cooldown_c);

    // SDL_Log("spawning %i, cooldown: %i", number_per_spawn, cooldown);
  }
};

void
update_spawner_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, survive_e, survive_c);
  GET_FIRST_OR_RETURN(SINGLE_OnDiskSpawners, r, disk_spawn_data_e, disk_spawn_data_c);

  // dont update survive timer when theres a boss
  const bool boss_is_alive = r.view<const BossComponent>().size() > 0;
  if (boss_is_alive)
    return;

  // Update survive timer
  survive_c.time_left_cur -= dt;
  survive_c.time_left_cur = glm::max(survive_c.time_left_cur, 0.0f);

  // How many of each enemies do we currently have?
  const auto& enemies_view = r.view<const EnemyComponent, const ItemKey>();
  std::unordered_map<std::string, int> enemy_to_amount;
  for (const auto& [e, enemy_c, item_c] : enemies_view.each())
    enemy_to_amount[item_c.key] += 1;

  //
  // NOTE: if enemy spawner and wave spawner use the same key,
  // then there's a bug that the enemy spawner could spawn an enemy,
  // the number of enemies would update, but not in the enemy_to_amount map,
  // and the wave spawner would spawn because it hasnt updated.
  // this is currently fine as they spawn different enemy_key enemies
  //

  update_enemy_spawner(r, enemy_to_amount);
  update_wave_spawner(r, enemy_to_amount);
};

} // namespace game2d