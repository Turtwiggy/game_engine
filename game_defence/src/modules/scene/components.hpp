#pragma once

namespace game2d {

enum class Scene
{
  menu,
  duckgame_overworld,
  dungeon_designer,
  turnbasedcombat,
  minigame_bamboo,

  // old scenes, possibly remove
  test_scene_gun,

  count
};

struct SINGLETON_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d