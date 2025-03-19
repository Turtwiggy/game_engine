#include "pch.hpp"

#include "steam_input_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"

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

class ControllerEvents
{
public:
  ControllerEvents()
    : m_connected_callback(this, &ControllerEvents::OnControllerConnected)
    , m_disconnected_callback(this, &ControllerEvents::OnControllerDisconnected) {};
  ~ControllerEvents() {};

  STEAM_CALLBACK(ControllerEvents, OnControllerConnected, SteamInputDeviceConnected_t, m_connected_callback);
  STEAM_CALLBACK(ControllerEvents, OnControllerDisconnected, SteamInputDeviceDisconnected_t, m_disconnected_callback);
};

void
ControllerEvents::OnControllerConnected(SteamInputDeviceConnected_t* pCallback)
{
  InputHandle_t disconnectedControllerHandle = pCallback->m_ulConnectedDeviceHandle;
  SDL_Log("(callback) Controller connect event.");
}

void
ControllerEvents::OnControllerDisconnected(SteamInputDeviceDisconnected_t* pCallback)
{
  InputHandle_t disconnectedControllerHandle = pCallback->m_ulDisconnectedDeviceHandle;
  SDL_Log("(callback) Controller disconnect event.");
}

void
init_steam_input_actions(entt::registry& r)
{
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  auto& digital_action_handles = steam_c.digital_action_handles;
  auto& analog_action_handles = steam_c.analog_action_handles;
  auto& action_set_handles = steam_c.action_set_handles;

  digital_action_handles[(int)DA::Game_Up] = SteamInput()->GetDigitalActionHandle("action_up");
  digital_action_handles[(int)DA::Game_Down] = SteamInput()->GetDigitalActionHandle("action_down");
  digital_action_handles[(int)DA::Game_Left] = SteamInput()->GetDigitalActionHandle("action_left");
  digital_action_handles[(int)DA::Game_Right] = SteamInput()->GetDigitalActionHandle("action_right");
  digital_action_handles[(int)DA::Game_Pause] = SteamInput()->GetDigitalActionHandle("action_pause");
  digital_action_handles[(int)DA::Game_Select] = SteamInput()->GetDigitalActionHandle("action_select");
  digital_action_handles[(int)DA::Game_Cancel] = SteamInput()->GetDigitalActionHandle("action_cancel");
  digital_action_handles[(int)DA::Game_Shoot] = SteamInput()->GetDigitalActionHandle("action_shoot");
  digital_action_handles[(int)DA::Game_Ability1] = SteamInput()->GetDigitalActionHandle("action_ability1");
  digital_action_handles[(int)DA::Game_Ability2] = SteamInput()->GetDigitalActionHandle("action_ability2");

  // digital_action_handles[(int)DA::Menu_Up] = SteamInput()->GetDigitalActionHandle("menu_up");
  // digital_action_handles[(int)DA::Menu_Down] = SteamInput()->GetDigitalActionHandle("menu_down");
  // digital_action_handles[(int)DA::Menu_Left] = SteamInput()->GetDigitalActionHandle("menu_left");
  // digital_action_handles[(int)DA::Menu_Right] = SteamInput()->GetDigitalActionHandle("menu_right");
  // digital_action_handles[(int)DA::Menu_Pause] = SteamInput()->GetDigitalActionHandle("menu_pause");
  // digital_action_handles[(int)DA::Menu_Select] = SteamInput()->GetDigitalActionHandle("menu_select");
  // digital_action_handles[(int)DA::Menu_Cancel] = SteamInput()->GetDigitalActionHandle("menu_cancel");

  analog_action_handles[(int)AA::LAnalogControls] = SteamInput()->GetAnalogActionHandle("l_analog");
  analog_action_handles[(int)AA::RAnalogControls] = SteamInput()->GetAnalogActionHandle("r_analog");

  action_set_handles[(int)AS::ActionSet_GameControls] = SteamInput()->GetActionSetHandle("game_controls");
  // action_set_handles[(int)AS::ActionSet_MenuControls] = SteamInput()->GetActionSetHandle("menu_controls");

  // Action set layer handle
  // m_ControllerActionSetHandles[actionSet_Layer_Thrust] = SteamInput()->GetActionSetHandle( "thrust_action_layer" );

  // clang-format on

  // Each controller will generate a device connected event.
  SteamAPI_ISteamInput_EnableDeviceCallbacks(SteamAPI_SteamInput());
  static ControllerEvents eve; // Gotta be a better way?
};

void
init_steam_input(entt::registry& r)
{
  // when Init(true): update explicitely with a separate call
  // when Init(false): update when SteamAPI_RunCallbacks() is called
  if (!SteamInput()->Init(false)) {
    SDL_Log("Fatal Error, SteamInput()->Init() failed");
    exit(1);
  }
  SDL_Log("SteamInput()->Init() Success");

  //
  // Steam uses the action file in this location:
  // C:\Software\Steam\controller_config\{APPID}.vdf
  //
  // To generate configs in this location via the controller configurator
  // C:\Software\Steam\steamapps\common\Steam Controller Configs\{SOMENUMBER}\config\{APPID}\YOURCONFIG.vdf
  //
  // IF you want to bundle controller configs in the app (i.e. uncomment the below)
  // you need a e.g. steam_input_manifest.vdf
  // with a "configuration {}" section, that links the generated
  // e.g. steam_input_manifest_steam.vdf
  //
  // ALTERNATIVELY: upload your generated config via
  // steamworkshop, and dont bundle a .vdf with the app
  //
  // const auto path = std::filesystem::absolute(".");
  // const auto abs_path_to_vdf = path.generic_string() + "/steam_input_manifest.vdf";
  // SDL_Log("Absolute path: %s", abs_path_to_vdf.c_str());
  // auto result = SteamInput()->SetInputActionManifestFilePath(abs_path_to_vdf.c_str());
  // if (!result) {
  //   SDL_Log(".vdf file not found or corrupt");
  //   exit(1);
  // }

  create_persistent<SINGLE_SteamControllers>(r);
  init_steam_input_actions(r);
};

void
update_steam_input_handles(entt::registry& r)
{
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  steam_c.n_active = SteamInput()->GetConnectedControllers(steam_c.handles.data());
};

void
generate_button_state(SINGLE_SteamControllers& steam_c, InputHandle_t handle)
{
  const auto you_held = steam_c.last_frame_held[handle]; // copy

  // set all as unheld
  steam_c.last_frame_held[handle].clear();

  std::vector<DA> newly_down;
  std::vector<DA> newly_released;

  for (int i = 0; i < static_cast<int>(DA::count); i++) {
    const auto act = magic_enum::enum_cast<DA>(i).value();

    // Get the current state of the button
    const bool held = controller_button_held(steam_c, handle, act);
    const bool held_last_frame = std::find(you_held.begin(), you_held.end(), act) != you_held.end();

    // Generate button down events.
    if (held && !held_last_frame)
      newly_down.push_back(act);

    // Generate button release events.
    if (!held && held_last_frame)
      newly_released.push_back(act);

    // Now, set the button as held
    if (held)
      steam_c.last_frame_held[handle].push_back(act);
  }

  steam_c.this_frame_down[handle] = newly_down;
  steam_c.this_frame_release[handle] = newly_released;
};

void
update_steam_input(entt::registry& r)
{
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);

  // check connect/disconnects
  update_steam_input_handles(r);

  // Generate button down states for all the handles.
  steam_c.this_frame_down.clear();
  for (int h = 0; h < steam_c.n_active; h++)
    generate_button_state(steam_c, steam_c.handles[h]);

  set_all_steam_controller_action_set(steam_c, AS::ActionSet_GameControls);
};

bool
controller_button_down(const SINGLE_SteamControllers& steam_c, InputHandle_t handle, const DA dwAction)
{
  if (handle == 0)
    return false;
  const std::unordered_map<InputHandle_t, std::vector<DA>>& all_down = steam_c.this_frame_down;

  if (!all_down.contains(handle))
    return false;
  const auto& you_down = all_down.at(handle);

  const auto it = std::find(you_down.begin(), you_down.end(), dwAction);
  return it != std::end(you_down);
};

bool
controller_button_held(const SINGLE_SteamControllers& steam_c, const InputHandle_t handle, const DA dwAction)
{
  if (handle == 0)
    return false;

  auto& digital_action_handles = steam_c.digital_action_handles;

  auto h = digital_action_handles[(int)dwAction];
  ControllerDigitalActionData_t data = SteamAPI_ISteamInput_GetDigitalActionData(SteamAPI_SteamInput(), handle, h);

  // Actions are only 'active' when they're assigned to a control in an action set,
  // and that action set is active.
  if (data.bActive)
    return data.bState;

  return false;
};

bool
controller_button_release(const SINGLE_SteamControllers& steam_c, InputHandle_t handle, const DA dAction)
{
  if (handle == 0)
    return false;
  const std::unordered_map<InputHandle_t, std::vector<DA>>& all_release = steam_c.this_frame_release;
  if (!all_release.contains(handle))
    return false;
  const auto& you_release = all_release.at(handle);

  const auto it = std::find(you_release.begin(), you_release.end(), dAction);
  return it != std::end(you_release);
}

glm::vec2
controller_axis(entt::registry& r, InputHandle_t handle, AA aAction)
{
  if (handle == 0)
    return { 0, 0 };

  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& analog_action_handles = steam_c.analog_action_handles;

  auto h = analog_action_handles[(int)aAction];

  ControllerAnalogActionData_t data = SteamAPI_ISteamInput_GetAnalogActionData(SteamAPI_SteamInput(), handle, h);

  if (data.bActive)
    return { data.x, data.y };

  return { 0, 0 };
};

void
set_steam_controller_action_set(SINGLE_SteamControllers& steam_c, InputHandle_t handle, AS set)
{
  if (handle == 0)
    return;

  const auto& sets = steam_c.action_set_handles;

  // This call is low-overhead and can be called repeatedly from game code that is active in a specific mode.
  SteamInput()->ActivateActionSet(handle, sets[(int)set]);
};

void
set_all_steam_controller_action_set(SINGLE_SteamControllers& steam_c, AS set)
{
  for (int j = 0; j < steam_c.n_active; j++) {
    auto handle = steam_c.handles[j];
    set_steam_controller_action_set(steam_c, handle, set);
  }
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

std::vector<InputHandle_t>
non_zero_handles(const std::vector<InputHandle_t>& handles)
{
  auto non_zero = [](const InputHandle_t h) { return h != 0; };
  auto non_zero_handles = handles | std::views::filter(non_zero);
  return std::vector(non_zero_handles.begin(), non_zero_handles.end());
};

} // namespace game2d