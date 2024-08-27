#pragma once

namespace game2d {

enum class Scene
{
  splashscreen,
  menu,
  overworld_revamped,
  dungeon_designer,
  minigame_bamboo,

  test, // used by tests
  count
};

struct SINGLETON_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d