#pragma once

namespace game2d {

enum class LevelEditorMode
{
  edit,
  play,

  count,
};

struct SINGLETON_LevelEditor
{
  LevelEditorMode mode = LevelEditorMode::edit;
};

} // namespace game2d