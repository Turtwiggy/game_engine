#include "pch.hpp"

#include "drop_gold_helpers.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/systems/system_create_item/create_item_components.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"

namespace game2d {

void
handle_death_event__drop_gold(entt::registry& r, const DeathEvent& evt)
{
  if (!r.all_of<EnemyComponent>(evt.dead))
    return;

  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);
  gold_c.enemies_to_kill_before_next_drop_cur--;

  if (gold_c.enemies_to_kill_before_next_drop_cur <= 0) {
    gold_c.enemies_to_kill_before_next_drop_cur = gold_c.enemies_to_kill_before_next_drop_max;

    // drop some gold
    CreateItemRequest req;
    req.item = "item_gold";
    req.position = get_position(r, evt.dead);
    create_empty<CreateItemRequest>(r, req);
  }
}

} // namespace game2d