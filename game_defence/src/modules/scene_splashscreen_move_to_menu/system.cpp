#include "system.hpp"

#include "components.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "modules/scene/helpers.hpp"

namespace game2d {

void
update_scene_splashscreen_move_to_menu_system(entt::registry& r, const float dt)
{
  auto& data = get_first_component<SINGLE_SplashScreen>(r);
  const auto& input = get_first_component<SINGLE_InputComponent>(r);

  const auto audio_e = get_first<SINGLE_AudioComponent>(r);
  if (audio_e == entt::null)
    return;

  const auto& audio = get_first_component<SINGLE_AudioComponent>(r);
  if (!audio.loaded)
    return; // wait for sounds to be loaded

  // After X seconds, move to menu
  data.time_on_splashscreen_seconds -= dt;
  if (data.time_on_splashscreen_seconds <= 0.0f || input.unprocessed_inputs.size() > 0)
    move_to_scene_start(r, Scene::menu);
};

} // namespace game2d