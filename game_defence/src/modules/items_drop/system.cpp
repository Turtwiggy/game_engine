#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/combat_damage/components.hpp"

namespace game2d {

void
update_intent_drop_item_system(entt::registry& r)
{

  // enemies drop some xp
  const auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  for (const auto& dead : dead.dead) {
    const auto* team = r.try_get<TeamComponent>(dead);
    if (team && team->team == AvailableTeams::enemy)
      r.emplace<WantsToDrop>(dead);
  }

  // WantsToDrop
  const auto& view = r.view<const TransformComponent, WantsToDrop>();
  for (const auto& [entity, transform, request] : view.each()) {

    CreateEntityRequest req;
    req.type = EntityType::actor_pickup_xp;
    req.position = transform.position;
    r.emplace<CreateEntityRequest>(r.create(), req);

    // request is done... but entity will die next frame anyway
    r.remove<WantsToDrop>(entity);
  }
};

} // namespace game2d