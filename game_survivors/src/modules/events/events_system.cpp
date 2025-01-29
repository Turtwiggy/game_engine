#include "events_system.hpp"
#include "engine/entt/helpers.hpp"

#include "events_components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_helpers.hpp"
#include "modules/event_coll_player_enemy/event_coll_player_enemy_helpers.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_helpers.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/event_shoot/event_shoot_components.hpp"
#include "modules/event_trait_assassin/trait_assassin_helpers.hpp"
#include "modules/event_trait_fanfire/trait_fanfire_helpers.hpp"
#include "modules/event_trait_splinter/trait_splinter_helpers.hpp"
#include "modules/event_upgrade/event_upgrade_helpers.hpp"

namespace game2d {

static entt::dispatcher dispatcher;

void
init_events_system(entt::registry& r)
{
  create_persistent<SINGLE_Events>(r, SINGLE_Events{ &dispatcher });
  auto& ed = get_first_component<SINGLE_Events>(r);

  // link event => function
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_bullet_other_coll>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_xp>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enemy_coll_enter>(r);
  ed.dispatcher->sink<OnCollisionExit>().connect<&handle_player_enemy_coll_exit>(r);

  // ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event_for_ui>(r);
  ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event_take_damage>(r);
  ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event__trait_assassin>(r);

  ed.dispatcher->sink<ShootEvent>().connect<&handle_shoot_event__trait_fanfire>(r);

  ed.dispatcher->sink<DeathEvent>().connect<&handle_death_event__trait_splinter>(r);

  ed.dispatcher->sink<UpgradeEvent>().connect<&handle_upgrade_event>(r);
}

void
update_events_system(entt::registry& r)
{
  const auto dispatcher_e = get_first<SINGLE_Events>(r);
  if (dispatcher_e == entt::null)
    return;
  auto& ed = r.get<SINGLE_Events>(dispatcher_e);
  ed.dispatcher->update(); // dispatch events
};

} // namespace game2d