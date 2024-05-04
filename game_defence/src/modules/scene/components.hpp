#pragma once

namespace game2d {

enum class Scene
{
  menu,
  test_scene_gun,
  game,
  spaceship_designer,
  duckgame,
  dungeon,

  count
};

struct SINGLETON_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d