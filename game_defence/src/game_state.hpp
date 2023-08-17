#pragma once

namespace game2d {

enum class GameState
{
  RUNNING,
  PAUSED,
};

struct SINGLETON_GameStateComponent
{
  GameState state = GameState::RUNNING;
};

} // namespace game2d