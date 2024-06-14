#pragma once

namespace game2d {

enum class Scene
{
  menu,
  overworld,
  dungeon_designer,
  turnbasedcombat,
  minigame_bamboo,

  count
};

struct SINGLETON_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d