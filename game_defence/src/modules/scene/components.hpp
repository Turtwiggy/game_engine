#pragma once

namespace game2d {

enum class Scene
{
  menu,
  game,

  count
};

struct SINGLETON_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d