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

// TODO: put this where it belongs
struct SINGLETON_Wave
{
  int wave = 1;
};

} // namespace game2d