#include "event_player_coll_item_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/resolve_collisions/resolve_collisions_helpers.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

namespace game2d {

void
handle_player_enter_item(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [player_e, item_e] = collision_of_interest<PlayerComponent, DefaultInventory>(r, evt.a, evt.b);
  if (player_e == entt::null || item_e == entt::null)
    return;

  auto& coll = r.get_or_emplace<CollInfo>(player_e);
  coll.other.emplace(item_e);
};

void
handle_player_exit_item(entt::registry& r, const OnCollisionExit& evt)
{
  const auto [player_e, item_e] = collision_of_interest<PlayerComponent, DefaultInventory>(r, evt.a, evt.b);
  if (player_e == entt::null || item_e == entt::null)
    return;

  auto& coll = r.get_or_emplace<CollInfo>(player_e);
  const auto& it = std::find(coll.other.begin(), coll.other.end(), item_e);
  if (it != coll.other.end())
    coll.other.erase(it);
};

} // namespace game2d