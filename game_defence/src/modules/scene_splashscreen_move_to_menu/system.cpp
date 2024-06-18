#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/scene/helpers.hpp"

namespace game2d {

void
update_scene_splashscreen_move_to_menu_system(entt::registry& r, const float dt)
{
  auto& data = get_first_component<SINGLE_SplashScreen>(r);

  // After X seconds, move to menu
  data.time_on_splashscreen_seconds -= dt;
  if (data.time_on_splashscreen_seconds <= 0.0f) {
    move_to_scene_start(r, Scene::menu);
  }
};

} // namespace game2d