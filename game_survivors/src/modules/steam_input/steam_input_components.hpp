#pragma once

#include <steam/isteaminput.h>
#include <steam/steam_api.h>

#include <vector>

namespace game2d {

#define STEAM_CONTROLLER_MAX_COUNT 16

enum class DigitalAction
{
  Game_Up = 0,
  Game_Down,
  Game_Left,
  Game_Right,
  Game_Shoot,
  Game_Cancel,
  Game_Menu,
  Game_Join,

  Menu_Up,
  Menu_Down,
  Menu_Left,
  Menu_Right,
  Menu_Select,
  Menu_Cancel,
  Menu_JoinSlot,
  Menu_LeaveSlot,

  count,
};

enum class AnalogAction
{
  LAnalogControls = 0,
  RAnalogControls,

  count,
};

enum class ActionSet
{
  ActionSet_GameControls = 0,
  ActionSet_MenuControls,

  count,
};

using DA = DigitalAction;
using AA = AnalogAction;
using AS = ActionSet;

struct SINGLE_SteamControllers
{
  InputDigitalActionHandle_t digital_action_handles[(int)DA::count];
  InputAnalogActionHandle_t analog_action_handles[(int)AA::count];
  InputActionSetHandle_t action_set_handles[(int)AS::count];

  int n_active = 0;
  std::vector<InputHandle_t> handles;

  SINGLE_SteamControllers()
  {
    handles.resize(STEAM_CONTROLLER_MAX_COUNT);

    for (int i = 0; i < static_cast<int>(DA::count); i++)
      digital_action_handles[i] = 0;
    for (int i = 0; i < static_cast<int>(AA::count); i++)
      analog_action_handles[i] = 0;
    for (int i = 0; i < static_cast<int>(AS::count); i++)
      action_set_handles[i] = 0;

    // active_controller_handle = 0;
  }
};

// attach this to a player
struct SteamControllerComponent
{
  InputHandle_t handle = 0;
};

} // namespace game2d