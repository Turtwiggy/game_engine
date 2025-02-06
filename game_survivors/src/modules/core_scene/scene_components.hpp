#pragma once

namespace game2d {

enum class Scene
{
  splashscreen,
  menu,
  select,
  survive,
  spritestack,

  test, // used by tests
  count
};

struct SINGLE_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d