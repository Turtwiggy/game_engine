#include "steam_input_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"

#include <filesystem>
#include <steam/steam_api.h>
#include <steam/steam_api_flat.h>

namespace game2d {

// callbacks:
// SteamInputConfigurationLoaded_t
// SteamInputDeviceConnected_t
// SteamInputDeviceDisconnected_t
// SteamInputGamepadSlotChange_t

// Use
// GetActionSetHandle
// GetDigitalActionHandle
// GetAnalogActionHandle
// to resolve the action & set names in to handles.

// Step 1: Creating an in-game actions file
// Step 2: Creating a default configuration
// Step 3.1: The steam input api
// https://partner.steamgames.com/doc/features/steam_controller/getting_started_for_devs#implement_steam_input

void
init_steam_input_actions(entt::registry& r)
{
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  auto& digital_action_handles = steam_c.digital_action_handles;
  auto& analog_action_handles = steam_c.analog_action_handles;
  auto& action_set_handles = steam_c.action_set_handles;

  // digital_action_handles[Action_GameUp] = SteamInput()->GetDigitalActionHandle("action_up");
  // digital_action_handles[Action_GameDown] = SteamInput()->GetDigitalActionHandle("action_down");
  // digital_action_handles[Action_GameLeft] = SteamInput()->GetDigitalActionHandle("action_left");
  // digital_action_handles[Action_GameRight] = SteamInput()->GetDigitalActionHandle("action_right");
  // digital_action_handles[Action_GameCancel] = SteamInput()->GetDigitalActionHandle("action_cancel");
  // digital_action_handles[Action_GameMenu] = SteamInput()->GetDigitalActionHandle("action_menu");

  // digital_action_handles[Menu_Up] = SteamInput()->GetDigitalActionHandle("menu_up");
  // digital_action_handles[Menu_Down] = SteamInput()->GetDigitalActionHandle("menu_down");
  // digital_action_handles[Menu_Left] = SteamInput()->GetDigitalActionHandle("menu_left");
  // digital_action_handles[Menu_Right] = SteamInput()->GetDigitalActionHandle("menu_right");
  // digital_action_handles[Menu_Select] = SteamInput()->GetDigitalActionHandle("menu_select");
  // digital_action_handles[Menu_Cancel] = SteamInput()->GetDigitalActionHandle("menu_cancel");
  // clang-format off

  digital_action_handles[(int)DA::eControllerDigitalAction_TurnLeft] = SteamInput()->GetDigitalActionHandle("turn_left");
  digital_action_handles[(int)DA::eControllerDigitalAction_TurnRight] = SteamInput()->GetDigitalActionHandle("turn_right");
  digital_action_handles[(int)DA::eControllerDigitalAction_ForwardThrust] = SteamInput()->GetDigitalActionHandle("forward_thrust");
  digital_action_handles[(int)DA::eControllerDigitalAction_ReverseThrust] = SteamInput()->GetDigitalActionHandle("backward_thrust");
  digital_action_handles[(int)DA::eControllerDigitalAction_FireLasers] = SteamInput()->GetDigitalActionHandle("fire_lasers");
  digital_action_handles[(int)DA::eControllerDigitalAction_PauseMenu] = SteamInput()->GetDigitalActionHandle("pause_menu");

  digital_action_handles[(int)DA::eControllerDigitalAction_MenuUp] = SteamInput()->GetDigitalActionHandle("menu_up");
  digital_action_handles[(int)DA::eControllerDigitalAction_MenuDown] = SteamInput()->GetDigitalActionHandle("menu_down");
  digital_action_handles[(int)DA::eControllerDigitalAction_MenuLeft] = SteamInput()->GetDigitalActionHandle("menu_left");
  digital_action_handles[(int)DA::eControllerDigitalAction_MenuRight] = SteamInput()->GetDigitalActionHandle("menu_right");
  digital_action_handles[(int)DA::eControllerDigitalAction_MenuSelect] = SteamInput()->GetDigitalActionHandle("menu_select");
  digital_action_handles[(int)DA::eControllerDigitalAction_MenuCancel] = SteamInput()->GetDigitalActionHandle("menu_cancel");

  // analog_action_handles[AnalogAction_Move] = SteamInput()->GetAnalogActionHandle("AnalogControls");
  analog_action_handles[(int)AA::eControllerAnalogAction_AnalogControls] = SteamInput()->GetAnalogActionHandle("analog_controls");

  // action_set_handles[ActionSet_GameControls] = SteamInput()->GetActionSetHandle("game_controls");
  // action_set_handles[ActionSet_MenuControls] = SteamInput()->GetActionSetHandle("menu_controls");
  action_set_handles[(int)AS::eControllerActionSet_ShipControls] = SteamInput()->GetActionSetHandle("ship_controls");
  action_set_handles[(int)AS::eControllerActionSet_MenuControls] = SteamInput()->GetActionSetHandle("menu_controls");

  // Action set layer handle
  // m_ControllerActionSetHandles[actionSet_Layer_Thrust] = SteamInput()->GetActionSetHandle( "thrust_action_layer" );

  // clang-format on
};

void
init_steam_input(entt::registry& r)
{
  // when Init(true): update explicitely with a seperate call
  // when Init(false): update when SteamAPI_RunCallbacks() is called
  SteamInput()->Init(false);

  const auto path = std::filesystem::absolute(".");
  const auto abs_path_to_vdf = path.generic_string() + "/steam_input_manifest.vdf";
  SDL_Log("Absolute path: %s", path.generic_string().c_str());

  auto result = SteamInput()->SetInputActionManifestFilePath(abs_path_to_vdf.c_str());
  if (!result) {
    SDL_Log(".vdf file not found or corrupt");
    exit(1);
  }

  create_persistent<SINGLE_SteamControllers>(r);
  init_steam_input_actions(r);
};

void
find_active_steam_input_device(entt::registry& r)
{
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);

  // Use the first available steam controller for all interaction. We can call this each frame to handle
  // a controller disconnecting and a different one reconnecting. Handles are guaranteed to be unique for
  // a given controller, even across power cycles.

  steam_c.n_active = SteamInput()->GetConnectedControllers(steam_c.handles.data());
};

void
update_steam_input(entt::registry& r)
{
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);

  // check connect/disconnects
  find_active_steam_input_device(r);
};

bool
controller_button_held(SINGLE_SteamControllers steam_c, InputHandle_t handle, DA dwAction)
{
  auto& digital_action_handles = steam_c.digital_action_handles;

  auto h = digital_action_handles[(int)dwAction];
  ControllerDigitalActionData_t data = SteamAPI_ISteamInput_GetDigitalActionData(SteamAPI_SteamInput(), handle, h);

  // Actions are only 'active' when they're assigned to a control in an action set,
  // and that action set is active.
  if (data.bActive)
    return data.bState;

  return false;
};

glm::vec2
controller_axis(entt::registry& r, InputHandle_t handle, AA aAction)
{
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& analog_action_handles = steam_c.analog_action_handles;

  auto h = analog_action_handles[(int)aAction];

  ControllerAnalogActionData_t data = SteamAPI_ISteamInput_GetAnalogActionData(SteamAPI_SteamInput(), handle, h);

  if (data.bActive)
    return { data.x, data.y };

  return { 0, 0 };
};

void
set_steam_controller_action_set(entt::registry& r, InputHandle_t handle, AS set)
{
  if (handle == 0)
    return;
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& sets = steam_c.action_set_handles;

  // This call is low-overhead and can be called repeatedly from game code that is active in a specific mode.
  SteamInput()->ActivateActionSet(handle, sets[(int)set]);
};

void
activate_steam_controller_action_set_layer(entt::registry& r, InputHandle_t handle, AS set_layer)
{
  if (handle == 0)
    return;
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& sets = steam_c.action_set_handles;

  SteamInput()->ActivateActionSetLayer(handle, sets[(int)set_layer]);
};

void
deactivate_steam_controller_action_set_layer(entt::registry& r, InputHandle_t handle, AS set_layer)
{
  if (handle == 0)
    return;
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& sets = steam_c.action_set_handles;

  SteamInput()->DeactivateActionSetLayer(handle, sets[(int)set_layer]);
};

bool
is_action_set_layer_active(entt::registry& r, InputHandle_t handle, AS set_layer)
{
  if (handle == 0)
    return false;
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& sets = steam_c.action_set_handles;

  ControllerActionSetHandle_t pActionSetLayerHandles[32];
  int nActiveLayerCount = SteamInput()->GetActiveActionSetLayers(handle, pActionSetLayerHandles);

  for (int i = 0; i < nActiveLayerCount; i++) {
    if (pActionSetLayerHandles[i] == sets[(int)set_layer])
      return true;
  }

  return false;
};

} // namespace game2d