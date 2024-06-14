#include "system.hpp"

#include "components.hpp"

#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

void
update_screenshake_system(entt::registry& r, const float timer, const float dt)
{
  // enable system if needed
  const auto screenshake_e = get_first<SINGLE_ScreenshakeComponent>(r);
  if (screenshake_e == entt::null)
    return;
  auto& screenshake = get_first_component<SINGLE_ScreenshakeComponent>(r);

  //
  // process requests
  //
  const auto& view = r.view<const RequestScreenshakeComponent>();
  for (const auto& [e, req] : view.each()) {
    screenshake.time_left += 0.025f;
  }
  r.destroy(view.begin(), view.end()); // done requests

  //
  // Do the actual screenshake
  //
  const bool do_screenshake = screenshake.time_left > 0;
  screenshake.time_left -= dt;
  screenshake.time_left = glm::max(screenshake.time_left, 0.0f);

  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  ri.instanced.bind();

  // cache the value to avoid set_bool
  static bool shake = do_screenshake;
  if (shake != do_screenshake) {
    shake = do_screenshake;
    ri.instanced.set_bool("shake", do_screenshake);
    ri.instanced.set_float("strength", 0.002f);
  }

  ri.instanced.set_float("time", timer);
};

} // namespace game2d