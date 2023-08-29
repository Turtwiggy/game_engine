#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"

namespace game2d {

void
update_actor_pickup_zone_system(entt::registry& r)
{
  const auto& lifecycle = get_first_component<SINGLETON_EntityBinComponent>(r);

  for (const auto& e : lifecycle.created_this_frame) {
    // check if a player was made...!
    if (auto* player = r.try_get<PlayerComponent>(e)) {
      // create a pickup zone
      CreateEntityRequest req;
      req.type = EntityType::actor_pickup_zone;
      req.parent = e;
      r.emplace<CreateEntityRequest>(r.create(), req);
    }
  }
}

} // namespace game2d