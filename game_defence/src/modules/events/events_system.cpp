#include "events_system.hpp"
#include "engine/entt/helpers.hpp"

#include "events_components.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/event_player_coll_item/event_player_coll_item_helpers.hpp"
#include "modules/ui_combat_damage_numbers/ui_combat_damage_numbers_system.hpp"

namespace game2d {

static entt::dispatcher dispatcher;

void
init_events_system(entt::registry& r)
{
  create_persistent<SINGLE_Events>(r, SINGLE_Events{ &dispatcher });
  auto& ed = get_first_component<SINGLE_Events>(r);

  // link event => function
  ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event>(r);
  ed.dispatcher->sink<DamageEvent>().connect<&handle_damage_event_for_ui>(r);
  ed.dispatcher->sink<OnCollisionEnter>().connect<&handle_player_enter_item>(r);
  ed.dispatcher->sink<OnCollisionExit>().connect<&handle_player_exit_item>(r);
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