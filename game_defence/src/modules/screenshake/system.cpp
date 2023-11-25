#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_screenshake_system(entt::registry& r, float timer, float dt)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

  bool do_screenshake = false;
  const float time_screenshake = 0.1f;
  static float time_screenshake_left = 0.0f;

  //
  // Check if any player shot and shake the screen

  const auto& view =
    r.view<const PlayerComponent, const TransformComponent, const InputComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, player, player_transform, input] : view.each()) {
    if (input.shoot)
      time_screenshake_left = time_screenshake;
  }

  do_screenshake = time_screenshake_left > 0;
  time_screenshake_left -= dt;

  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  ri.instanced.bind();
  ri.instanced.set_bool("shake", do_screenshake);
  ri.instanced.set_float("time", timer);
};

} // namespace game2d