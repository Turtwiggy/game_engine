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
  procedural_snake,

  test, // used by tests
  count
};

struct SINGLE_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d