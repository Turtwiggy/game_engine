#pragma once

namespace game2d {

enum class Scene
{
  splashscreen,
  pressanykey,
  menu,
  select_modifiers,
  select_ships,
  survive,

  develop_waves,
  develop_snake,
  develop_islands,

  test, // used by tests
  count
};

struct SINGLE_CurrentScene
{
  Scene s = Scene::menu;

  static SINGLE_CurrentScene instance;
};

} // namespace game2d