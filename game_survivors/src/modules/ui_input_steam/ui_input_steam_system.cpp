#include "ui_input_steam_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"

#include "imgui.h"
#include "steam/isteaminput.h"
#include <SDL_keyboard.h>
#include <SDL_scancode.h>
#include <steam/steam_api.h>

namespace game2d {

using DA = DigitalAction;
using AA = AnalogAction;
using AS = ActionSet;

void
update_ui_steam_input_system(entt::registry& r)
{
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);

  ImGuiWindowFlags flags = 0;

  ImGui::Begin("Steam", NULL, flags);

  // TEMP: debug your steam name
  const char* name = SteamFriends()->GetPersonaName();
  ImGui::Text("Welcome, %s", name);

  // There's a bug where the action handles aren't non-zero until a config is done loading. Soon config
  // information will be available immediately. Until then try to init as long as the handles are invalid.
  const auto& digital_action_handles = steam_c.digital_action_handles;
  if (digital_action_handles[(int)DA::Action_GameUp] == 0) {
    init_steam_input_actions(r);
    ImGui::Text("DigitalActionHandles are not loaded...");
    ImGui::End();
    return;
  }

  bool escape_pressed = get_key_down(input_c, SDL_SCANCODE_ESCAPE);
  // m_pGameEngine->BIsControllerActionActive( eControllerDigitalAction_PauseMenu ) ||
  // m_pGameEngine->BIsControllerActionActive( eControllerDigitalAction_MenuCancel ) )

  ImGui::Text("ConnectedControllers: %i", steam_c.n_active);

  // auto& action_set_menu = steam_c.action_set_handles[(int)AS::eControllerActionSet_MenuControls];
  auto& action_set = steam_c.action_set_handles[(int)AS::eControllerActionSet_ShipControls];
  auto& handles = steam_c.handles;

  for (int i = 0; i < steam_c.n_active; i++) {
    SteamInput()->ActivateActionSet(handles[i], action_set);

    ImGui::Text("MenuLeft %i", controller_button_held(steam_c, handles[i], DA::eControllerDigitalAction_MenuLeft));
    ImGui::Text("MenuRight %i", controller_button_held(steam_c, handles[i], DA::eControllerDigitalAction_MenuRight));
    ImGui::Text("TurnLeft %i", controller_button_held(steam_c, handles[i], DA::eControllerDigitalAction_TurnLeft));
    ImGui::Text("TurnRight %i", controller_button_held(steam_c, handles[i], DA::eControllerDigitalAction_TurnRight));
    ImGui::Text("ForwardThrust %i", controller_button_held(steam_c, handles[i], DA::eControllerDigitalAction_ForwardThrust));
    ImGui::Text("ReverseThrust %i", controller_button_held(steam_c, handles[i], DA::eControllerDigitalAction_ReverseThrust));
    ImGui::Text("FireLasers %i", controller_button_held(steam_c, handles[i], DA::eControllerDigitalAction_FireLasers));
    ImGui::Text("PauseMenu %i", controller_button_held(steam_c, handles[i], DA::eControllerDigitalAction_PauseMenu));

    // ImGui::Text("Action_GameCancel %i", controller_button_held(steam_c, handles[i], Action_GameCancel));
    // ImGui::Text("Action_GameMenu %i", controller_button_held(steam_c, handles[i], Action_GameMenu));
    auto analog = controller_axis(r, handles[i], AA::eControllerAnalogAction_AnalogControls);
    ImGui::Text("Analog: %f %f", analog.x, analog.y);
  }

  ImGui::End();
}

} // namespace game2d