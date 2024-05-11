#pragma once

namespace game2d {

enum class Scene
{
  menu,
  game,
  duckgame_overworld,
  dungeon_designer,
  turnbasedcombat,

  test_scene_gun,
  count
};

struct SINGLETON_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d