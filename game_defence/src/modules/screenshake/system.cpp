#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_screenshake_system(entt::registry& r, const float timer, const float dt)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  auto& screenshake = get_first_component<SINGLE_ScreenshakeComponent>(r);

  bool do_screenshake = screenshake.time_left > 0;
  screenshake.time_left -= dt;
  screenshake.time_left = glm::max(screenshake.time_left, 0.0f);

  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  ri.instanced.bind();

  // cache the value to avoid set_bool
  static bool shake = do_screenshake;
  if (shake != do_screenshake) {
    shake = do_screenshake;
    ri.instanced.set_bool("shake", do_screenshake);
  }

  ri.instanced.set_float("time", timer);
};

} // namespace game2d