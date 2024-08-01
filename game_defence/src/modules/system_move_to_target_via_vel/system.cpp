#include "system.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_move_to_target_via_vel(entt::registry& r)
{
  //
  // Update physics bodies that have static targets to their static target
  //
  const auto& view = r.view<PhysicsBodyComponent, const StaticTargetComponent, const SetVelocityToTargetComponent>();
  for (const auto& [e, pb_c, target_c, req_c] : view.each()) {
    const auto a = get_position(r, e);

    if (!target_c.target.has_value())
      continue;
    const auto b = target_c.target.value();

    const glm::vec2 nrm_dir = engine::normalize_safe(b - a);

    pb_c.body->SetLinearVelocity({ nrm_dir.x * pb_c.base_speed, nrm_dir.y * pb_c.base_speed });
  }

  //
  // Update physics bodies that have dynamic targets to their dynamic target
  //
  const auto& view_dyn = r.view<PhysicsBodyComponent, DynamicTargetComponent, SetVelocityToTargetComponent>();
  for (const auto& [e, pb_c, target_c, req_c] : view_dyn.each()) {
    if (target_c.target == entt::null || !r.valid(target_c.target)) {
      r.remove<DynamicTargetComponent>(e); // your target died or became invalid
      continue;
    }

    const auto a = get_position(r, e);
    const auto b = get_position(r, target_c.target);
    const glm::vec2 nrm_dir = engine::normalize_safe(b - a);

    pb_c.body->SetLinearVelocity({ nrm_dir.x * pb_c.base_speed, nrm_dir.y * pb_c.base_speed });
  }
}

} // namespace game2d