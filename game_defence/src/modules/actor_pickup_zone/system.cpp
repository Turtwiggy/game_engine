#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"

namespace game2d {

void
update_actor_pickup_zone_system(entt::registry& r)
{
  for (const auto& [e, player, request] : r.view<PlayerComponent, GeneratePickupZoneComponent>().each()) {
    // create a pickup zone
    CreateEntityRequest req;
    req.type = EntityType::actor_pickup_zone;
    req.parent = e;
    r.emplace<CreateEntityRequest>(r.create(), req);

    // done with request
    r.remove<GeneratePickupZoneComponent>(e);
  }
}

} // namespace game2d