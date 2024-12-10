#include "actor_brawler_ai_system.hpp"

#include "engine/maths/maths.hpp"
#include "modules/actor_brawler/actor_brawler_components.hpp"

namespace game2d {

static engine::RandomState rnd(0);

void
update_actor_brawler_ai_system(entt::registry& r, const float dt)
{
  const auto& view = r.view<BrawlerAI>();
  for (const auto& [e, brawler_c] : view.each()) {
    brawler_c.time_between_punches_left -= dt;
    if (brawler_c.time_between_punches_left > 0.0f)
      continue;

    // act
    r.emplace_or_replace<WantsToPunch>(e);

    // set timer going again
    const auto min = brawler_c.time_between_punches_min;
    const auto max = brawler_c.time_between_punches_max;
    brawler_c.time_between_punches_left = engine::rand_det_s(rnd.rng, min, max);
  }
}

} // namespace game2d