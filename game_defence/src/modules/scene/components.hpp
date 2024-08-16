#pragma once

namespace game2d {

enum class Scene
{
  splashscreen,
  menu,
  overworld_revamped,

  dungeon_designer,
  turnbasedcombat,

  fov_tests,

  minigame_bamboo,

  count
};

struct SINGLETON_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d