#include "pch.hpp"

#include "anchor_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actors/actor_islander/islander_components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"


namespace game2d {

void
remove_anchor(entt::registry& r, entt::entity boat_e)
{
  auto& anchor_c = r.get<DroppedAnchorComponent>(boat_e);

  auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
  dead_c.dead.push_back(anchor_c.target_e);

  r.remove<DroppedAnchorComponent>(boat_e);
  r.remove<ApplyForceToDynamicTarget>(boat_e);
  r.remove<PhysicsDynamicTarget>(boat_e);
}

} // namespace game2d