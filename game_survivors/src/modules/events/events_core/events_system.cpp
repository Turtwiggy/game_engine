#include "events_system.hpp"

#include "engine/entt/helpers.hpp"
#include "events_components.hpp"
#include "modules/events/event_coll_bullet_other/event_coll_bullet_other_helpers.hpp"
#include "modules/events/event_coll_player_enemy/event_coll_player_enemy_helpers.hpp"
#include "modules/events/event_coll_player_gold/event_coll_player_gold_helpers.hpp"
#include "modules/events/event_coll_player_hp/event_coll_player_hp_helpers.hpp"
#include "modules/events/event_coll_player_sea_mine/event_coll_player_sea_mine_helpers.hpp"
#include "modules/events/event_coll_player_vacuum_orb/event_coll_player_vacuum_orb_helpers.hpp"
#include "modules/events/event_coll_player_xp/event_coll_player_xp_helpers.hpp"
#include "modules/events/event_damage/event_damage_helpers.hpp"
#include "modules/events/event_damage_lifesteal/lifesteal_helpers.hpp"
#include "modules/events/event_death_exploder_screenshake/exploder_screenshake.hpp"
#include "modules/events/event_death_treasure_enemy/treasure_enemy_death_helpers.hpp"
#include "modules/events/event_shoot/event_shoot_components.hpp"
#include "modules/events/event_trait_fanfire/trait_fanfire_helpers.hpp"
#include "modules/events/event_trait_splinter/trait_splinter_helpers.hpp"
#include "modules/events/event_upgrade_aquired/event_upgrade_aquired_helpers.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_components.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_helpers.hpp"
#include "modules/systems/system_stats/stats_helpers.hpp"

namespace game2d {

static entt::dispatcher dispatcher;

void
init_events_system(entt::registry& r)
{
  create_persistent<SINGLE_Events>(r, SINGLE_Events{ &dispatcher });
  auto& ed = get_first_component<SINGLE_Events>(r);

  // link event => function
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_bullet_other_coll>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_gold>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_hp>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_sea_mine>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_vacuum_orb>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_xp>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enemy_coll_enter>(r);
  ed.dispatcher->sink<OnCollisionExit>().connect<&handle_player_enemy_coll_exit>(r);

  // ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event_for_ui>(r);
  ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event_take_damage>(r);
  ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event_lifesteal>(r);

  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__trait_fanfire>(r);

  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__trait_splinter>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__exploder_screenshake>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__treasure_enemy>(r);
  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__update_stats>(r);

  ed.dispatcher->sink<UpgradeEvent>().connect<&handle_upgrade_event>(r);

  ed.dispatcher->sink<WeaponLevelReachedEvent>().connect<handle_weapon_level_reached_event>(r);
}

void
update_events_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto dispatcher_e = get_first<SINGLE_Events>(r);
  if (dispatcher_e == entt::null)
    return;
  auto& ed = r.get<SINGLE_Events>(dispatcher_e);
  ed.dispatcher->update(); // dispatch events

  // Call the callbacks for postfixedupdate callbacks
  GET_FIRST_OR_RETURN(SINGLE_PostFixedUpdateCallbacks, r, callbacks_e, callbacks_c);
  for (const auto& callback : callbacks_c.callbacks)
    callback(r);
  callbacks_c.callbacks.clear();
};

} // namespace game2d