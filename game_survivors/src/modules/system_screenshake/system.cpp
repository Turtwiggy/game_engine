#include "system.hpp"

#include "components.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/system_screenshake/components.hpp"

namespace game2d {

void
update_screenshake_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_ScreenshakeComponent, r, shake_e, shake_c);

  const auto view = r.view<const RequestScreenshakeComponent>();
  for (const auto& [e, req] : view.each())
    shake_c.time_left += 0.025f;
  r.destroy(view.begin(), view.end()); // done requests

  // do the screenshake
  const bool do_screenshake = shake_c.time_left > 0;
  shake_c.time_left -= dt;
  shake_c.time_left = glm::max(shake_c.time_left, 0.0f);

  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  ri.instanced.bind();

  // cache the value to avoid set_bool
  static bool shake = do_screenshake;
  if (shake != do_screenshake) {
    shake = do_screenshake;
    ri.instanced.set_bool("shake", do_screenshake);
    ri.instanced.set_float("strength", 0.002f);
  }
};

} // namespace game2d