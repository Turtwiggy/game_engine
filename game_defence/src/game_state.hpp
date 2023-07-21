#pragma once

namespace game2d {

enum class GameState
{
  START,
  RUNNING,
  PAUSED,
  GAMEOVER_LOSE,
  GAMEOVER_WIN,
};

struct SINGLETON_GameStateComponent
{
  GameState state = GameState::START;
};

} // namespace game2d