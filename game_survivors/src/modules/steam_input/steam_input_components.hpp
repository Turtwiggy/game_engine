#pragma once

#include <steam/steam_api.h>

#include <vector>

namespace game2d {

#define STEAM_CONTROLLER_MAX_COUNT 16

enum class DigitalAction : int
{
  Action_GameUp,
  Action_GameDown,
  Action_GameLeft,
  Action_GameRight,
  Action_GameCancel,
  Action_GameMenu,

  Menu_Up,
  Menu_Down,
  Menu_Left,
  Menu_Right,
  Menu_Select,
  Menu_Cancel,

  count
};

enum class AnalogAction
{
  // AnalogAction_Move,
  eControllerAnalogAction_AnalogControls,

  count
};

enum class ActionSet
{
  // ActionSet_GameControls,
  // ActionSet_MenuControls,
  eControllerActionSet_ShipControls,
  eControllerActionSet_MenuControls,
  eControllerActionSet_Layer_Thrust,

  count
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

  // Origins for all the Steam Input actions. The 'origin' is where the action is currently bound to,
  // ie 'jump' is currently bound to the Steam Controller 'A' button.
  // EInputActionOrigin digital_action_origins[DigitalAction_NumActions];
  // EInputActionOrigin analog_action_origins[AnalogAction_NumActions];

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

} // namespace game2d