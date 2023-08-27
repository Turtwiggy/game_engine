#pragma once

namespace game2d {

enum class LevelEditorMode
{
  place,
  play,

  count,
};

struct SINGLETON_LevelEditor
{
  LevelEditorMode mode = LevelEditorMode::place;
};

} // namespace game2d