#include "pch.hpp"

#include "island_ai_system.hpp"

#include "engine/maths/maths.hpp"
#include "island_ai_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"

namespace game2d {

void
update_island_ai_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  static engine::RandomState rnd_ai(0);

  const auto view =
    r.view<const MovementIslandComponent, InputComponent, IslanderAiComponent>(entt::exclude<PlayerComponent>);

  for (const auto& [e, island_c, input_c, islander_ai_c] : view.each()) {
    islander_ai_c.time_between_ai_checks_cur -= dt;
    if (islander_ai_c.time_between_ai_checks_cur > 0.0f)
      continue;

    const auto rnd_cd =
      engine::rand_det_s(rnd_ai.rng, islander_ai_c.time_between_ai_checks_min, islander_ai_c.time_between_ai_checks_max);
    islander_ai_c.time_between_ai_checks_cur = rnd_cd;

    // choose a random direction to move
    const auto rnd_dir = (int)engine::rand_det_s(rnd_ai.rng, 0, 4);

    if (rnd_dir == 0)
      input_c.dpad_u.push_back(ActionStateEnum::DOWN);
    else if (rnd_dir == 1)
      input_c.dpad_d.push_back(ActionStateEnum::DOWN);
    else if (rnd_dir == 2)
      input_c.dpad_l.push_back(ActionStateEnum::DOWN);
    else if (rnd_dir == 3)
      input_c.dpad_r.push_back(ActionStateEnum::DOWN);

    //
  }
}

} // namespace game2d