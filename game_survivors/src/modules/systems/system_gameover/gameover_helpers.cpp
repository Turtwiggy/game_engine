#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "gameover_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/systems/system_gameover/gameover_components.hpp"
#include "modules/ui/ui_gameover/ui_gameover_components.hpp"

namespace game2d {

bool
is_gameover(entt::registry& r)
{
  // gameover to only occur in survive scene
  const auto& scene_c = SINGLE_CurrentScene::instance;
  if (scene_c.s != Scene::survive)
    return false;

  auto over = false;

  // note: SINGLE_GameoverUI eats GameOverComponent,
  // when it turns the ui .open to true, so also check .open
  auto view = r.view<GameOverComponent>();
  over |= !view.empty();

  auto& gameover_ui = get_first_component<SINGLE_GameoverUI>(r);
  over |= gameover_ui.open;

  return over;
}

} // namespace game2d