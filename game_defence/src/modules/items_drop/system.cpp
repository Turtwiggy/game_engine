#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/lifecycle/components.hpp"

namespace game2d {

void
update_intent_drop_item_system(entt::registry& r)
{

  // enemies drop some xp
  const auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  for (const auto& dead : dead.dead) {
    const auto* team = r.try_get<TeamComponent>(dead);
    if (team && team->team == AvailableTeams::enemy) {
      WantsToDrop req;
      std::vector<entt::entity> items;

      // add an xp item
      // auto e = r.create();
      // r.emplace<EntityTypeComponent>(e, EntityType::actor_pickup_xp);
      // items.push_back(e);

      req.items = items;
      r.emplace<WantsToDrop>(dead, req);
    }
  }

  // WantsToDrop
  const auto& view = r.view<const TransformComponent, WantsToDrop>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, transform, request] : view.each()) {
    for (const auto& item : request.items) {

      const auto req = create_gameplay(r, r.get<EntityTypeComponent>(item).type);
      r.get<TransformComponent>(req).position = transform.position;
    }
  }

  // requests done...
  r.remove<WantsToDrop>(view.begin(), view.end());
};

} // namespace game2d
