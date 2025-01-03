#pragma once

namespace game2d {

enum class Scene
{
  splashscreen,
  menu,
  overworld,
  dungeon_designer,

  tutorial_shotgun_straight,
  tutorial_shotgun_diagonal,
  tutorial_knife_bleed,
  tutorial_hook_blackhole,

  test, // used by tests
  count
};

struct SINGLE_CurrentScene
{
  Scene s = Scene::menu;
};

} // namespace game2d