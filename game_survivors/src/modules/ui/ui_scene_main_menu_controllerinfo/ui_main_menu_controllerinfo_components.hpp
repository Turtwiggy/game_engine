#pragma once

#include <entt/fwd.hpp>
#include <steam/isteaminput.h>

namespace game2d {

// These are all the handles that want to play the game
// i.e. a subset of SINGLE_SteamConnectedControllers
struct SINGLE_SteamControllerGameState
{
  int players = 4;
  std::vector<InputHandle_t> handles_that_want_to_play;
  // std::vector<InputHandle_t> handles_that_are_deactivated;

  SINGLE_SteamControllerGameState()
  {
    //
    handles_that_want_to_play.resize(players, 0); // max is players
    //
  };
};

struct PlayerAnimData
{
  bool init = false;

  // randomly adjust mask strings
  float mask_min_rnd = 0.1f;
  float mask_max_rnd = 0.5f;

  // store the generated values
  float mask_string_l_y = 0.0f;
  float mask_string_r_y = 0.0f;

  // make the zzz bob
  float sleeping_mask_y_timer = 0.0f;
};

struct SINGLE_MainMenuAnimatedData
{
  std::vector<PlayerAnimData> data;
};

} // namespace game2d