#include "helpers.hpp"

#include "maths/maths.hpp"
#include "modules/actor_enemy_patrol/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_patrol_from_desc(entt::registry& r, const entt::entity& e, PatrolDescription& desc)
{
  static engine::RandomState rnd(0);

  // random speed
  auto& speed = r.get<VelocityComponent>(e);
  speed.base_speed = int(engine::rand_det_s(rnd.rng, desc.speed_min, desc.speed_max));

  // random patrol strength
  auto& patrol_c = r.get<PatrolComponent>(e);
  patrol_c.strength = int(engine::rand_det_s(rnd.rng, desc.units_min, desc.units_max));
};

} // namespace game2d