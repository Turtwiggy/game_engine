#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "gameover_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/systems/system_gameover/gameover_components.hpp"

namespace game2d {

bool
is_gameover(entt::registry& r)
{
  // gameover to only occur in survive scene
  const auto& scene_c = get_first_component<SINGLE_CurrentScene>(r);
  if (scene_c.s != Scene::survive)
    return false;

  auto view = r.view<GameOverComponent>();
  return view.size() > 0;
}

} // namespace game2d