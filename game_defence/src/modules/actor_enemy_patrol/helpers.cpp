#include "helpers.hpp"

#include "maths/maths.hpp"
#include "modules/actor_enemy_patrol/components.hpp"
#include "physics/components.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_patrol_from_desc(entt::registry& r, const entt::entity& e, PatrolDescription& desc)
{
  static engine::RandomState rnd(0);

  // random patrol strength
  auto& patrol_c = r.get<PatrolComponent>(e);
  patrol_c.strength = int(engine::rand_det_s(rnd.rng, desc.units_min, desc.units_max));

  // set speed inversly proportional to strength.
  // i.e. the stronger you are the slower you are
  const float min = desc.speed_min;
  const float max = desc.speed_max;
  const float t = engine::scale(patrol_c.strength, desc.units_max, desc.units_min, 0.0f, 1.0f);
  const int speed = static_cast<int>(glm::lerp(min, max, glm::clamp(t, 0.0f, 1.0f)));

  r.get<VelocityComponent>(e).base_speed = speed;
};

} // namespace game2d