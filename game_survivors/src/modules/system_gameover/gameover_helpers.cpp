#include "pch.hpp"

#include "gameover_helpers.hpp"
#include "modules/system_gameover/gameover_components.hpp"

namespace game2d {

bool
is_gameover(entt::registry& r)
{
  auto view = r.view<GameOverComponent>();
  return view.size() > 0;
}

} // namespace game2d