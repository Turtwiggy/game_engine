#include "events_system.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "events_components.hpp"
#include "modules/events/event_bump/bump_event_components.hpp"
#include "modules/events/event_bump_damage/bump_damage_event_helpers.hpp"
#include "modules/events/event_bump_drum/bump_drum_event_helpers.hpp"
#include "modules/events/event_bump_repair/bump_repair_event_helpers.hpp"
#include "modules/events/event_bump_revive/bump_revive_event_helpers.hpp"
#include "modules/events/event_coll_bullet_other/event_coll_bullet_other_helpers.hpp"
#include "modules/events/event_coll_flame_enemy/event_coll_flame_enemy_helpers.hpp"
#include "modules/events/event_coll_player_enemy/event_coll_player_enemy_helpers.hpp"
#include "modules/events/event_coll_player_gold/event_coll_player_gold_helpers.hpp"
#include "modules/events/event_coll_player_hp/event_coll_player_hp_helpers.hpp"
#include "modules/events/event_coll_player_island/event_coll_player_island.hpp"
#include "modules/events/event_coll_player_sea_mine/event_coll_player_sea_mine_helpers.hpp"
#include "modules/events/event_coll_player_vacuum_orb/event_coll_player_vacuum_orb_helpers.hpp"
#include "modules/events/event_coll_player_xp/event_coll_player_xp_helpers.hpp"
#include "modules/events/event_damage/event_damage_helpers.hpp"
#include "modules/events/event_damage_lifesteal/lifesteal_helpers.hpp"
#include "modules/events/event_death_exploder_screenshake/exploder_screenshake.hpp"
#include "modules/events/event_death_islander_give_xp/islander_death_give_xp_helpers.hpp"
#include "modules/events/event_death_islander_remove_from_island/islander_remove_from_island_helpers.hpp"
#include "modules/events/event_death_reassign_targets/reassign_targets_helpers.hpp"
#include "modules/events/event_death_spawn_revive_islander/spawn_revive_islander_helpers.hpp"
#include "modules/events/event_death_treasure_enemy/treasure_enemy_death_helpers.hpp"
#include "modules/events/event_island_to_boat/island_to_boat_helpers.hpp"
#include "modules/events/event_scene_changed_update_water_shader/update_water_shader_helpers.hpp"
#include "modules/events/event_shoot/event_shoot_components.hpp"
#include "modules/events/event_shoot_audio/shoot_audio_helpers.hpp"
#include "modules/events/event_shoot_autofire/event_shoot_autofire_helpers.hpp"
#include "modules/events/event_shoot_fireball/trait_shoot_fireball.hpp"
#include "modules/events/event_shoot_flamethrower/event_shoot_flamethrower.hpp"
#include "modules/events/event_shoot_island_turret/event_shoot_island_turret.hpp"
#include "modules/events/event_shoot_muzzleflash/event_shoot_muzzleflash.hpp"
#include "modules/events/event_shoot_to_deploy_turrets/event_shoot_turrets_helpers.hpp"
#include "modules/events/event_steam_remote_player_connected/event_steam_remote_player_connected_helpers.hpp"
#include "modules/events/event_trait_fanfire/trait_fanfire_helpers.hpp"
#include "modules/events/event_trait_splinter/trait_splinter_helpers.hpp"
#include "modules/events/event_upgrade_aquired/event_upgrade_aquired_helpers.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_components.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_helpers.hpp"
#include "modules/systems/system_audio_mix/audio_mix_system.hpp"
#include "modules/systems/system_stats/stats_helpers.hpp"

namespace game2d {

static entt::dispatcher dispatcher;

void
init_events_system(entt::registry& r)
{
  SINGLE_Events::instance = SINGLE_Events{ &dispatcher };
  auto& ed = SINGLE_Events::instance;

  // link event => function
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_bullet_other_coll>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_gold>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_hp>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_island>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_sea_mine>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_vacuum_orb>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_xp>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enemy_coll_enter>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_flame_enemy__coll_enter>(r);

  ed.dispatcher->sink<OnCollisionExit>().connect<&handle_player_enemy_coll_exit>(r);
  ed.dispatcher->sink<OnCollisionExit>().connect<&handle_flame_enemy__coll_exit>(r);

  // ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event_for_ui>(r);
  ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event_take_damage>(r);
  ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event_lifesteal>(r);

  ed.dispatcher->sink<BumpEvent>().connect<&handle_bump_event__damage>(r);
  ed.dispatcher->sink<BumpEvent>().connect<&handle_bump_event__drum>(r);
  ed.dispatcher->sink<BumpEvent>().connect<&handle_bump_event__repair>(r);
  ed.dispatcher->sink<BumpEvent>().connect<&handle_bump_event__revive>(r);

  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__audio>(r);
  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__autofire>(r);
  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__deploy_turrets>(r);
  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__trait_fanfire>(r);
  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__shoot_fireball>(r);
  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__muzzleflash>(r);
  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__flamethrower>(r);
  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__island_turret>(r);

  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__trait_splinter>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__exploder_screenshake>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__treasure_enemy>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__update_stats>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__islander_death_give_xp>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__islander_remove_from_island>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__spawn_revive_islander>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__reassign_targets>(r);

  ed.dispatcher->sink<AudioCompleteEvent>().connect<&handle_audio_complete_event__new_game_track>(r);

  ed.dispatcher->sink<UpgradeEvent>().connect<&handle_upgrade_event>(r);

  ed.dispatcher->sink<WeaponLevelReachedEvent>().connect<handle_weapon_level_reached_event>(r);

  ed.dispatcher->sink<IslandToBoatEvent>().connect<handle_island_to_boat_event__start_game>(r);

  ed.dispatcher->sink<SceneChangedEvent>().connect<handle_scene_changed_event__update_water_shader>(r);

  ed.dispatcher->sink<RemotePlayerConnectedEvent>().connect<handle_steam_event__remote_player_connected>(r);
}

void
update_events_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& evts_c = SINGLE_Events::instance;
  evts_c.dispatcher->update(); // dispatch events

  // Call the callbacks for postfixedupdate callbacks
  GET_FIRST_OR_RETURN(SINGLE_PostFixedUpdateCallbacks, r, callbacks_e, callbacks_c);
  for (const auto& callback : callbacks_c.callbacks)
    callback(r);
  callbacks_c.callbacks.clear();
};

} // namespace game2d