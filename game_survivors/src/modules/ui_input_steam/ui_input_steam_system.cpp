#include "ui_input_steam_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
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
using namespace engine;

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
  ImGui::Text("ConnectedControllers: %i", steam_c.n_active);

  // There's a bug where the action handles aren't non-zero until a config is done loading. Soon config
  // information will be available immediately. Until then try to init as long as the handles are invalid.
  const auto& digital_action_handles = steam_c.digital_action_handles;
  if (digital_action_handles[(int)DA::Action_GameUp] == 0) {
    init_steam_input_actions(r);
    ImGui::Text("DigitalActionHandles are not loaded...");
    ImGui::End();
    return;
  }

  static auto mode = ActionSet::ActionSet_GameControls;
  static auto modes = engine::enum_class_to_vec_str<ActionSet>();
  WomboComboIn combo_in(modes);
  combo_in.label = "ActionSet";
  combo_in.current_index = static_cast<int>(mode);
  WomboComboOut combo_out = draw_wombo_combo(combo_in);
  if (combo_in.current_index != combo_out.selected)
    mode = static_cast<AS>(combo_out.selected);

  bool escape_pressed = get_key_down(input_c, SDL_SCANCODE_ESCAPE);
  // m_pGameEngine->BIsControllerActionActive( eControllerDigitalAction_PauseMenu ) ||
  // m_pGameEngine->BIsControllerActionActive( eControllerDigitalAction_MenuCancel ) )

  // auto& action_set_menu = steam_c.action_set_handles[(int)AS::eControllerActionSet_MenuControls];
  auto& action_set = steam_c.action_set_handles[(int)combo_out.selected];
  auto& handles = steam_c.handles;

  for (int i = 0; i < steam_c.n_active; i++) {
    SteamInput()->ActivateActionSet(handles[i], action_set);

    ImGui::Text("Action_GameUp %i", controller_button_held(steam_c, handles[i], DA::Action_GameUp));
    ImGui::Text("Action_GameDown %i", controller_button_held(steam_c, handles[i], DA::Action_GameDown));
    ImGui::Text("Action_GameLeft %i", controller_button_held(steam_c, handles[i], DA::Action_GameLeft));
    ImGui::Text("Action_GameRight %i", controller_button_held(steam_c, handles[i], DA::Action_GameRight));
    ImGui::Text("Action_GameCancel %i", controller_button_held(steam_c, handles[i], DA::Action_GameCancel));
    ImGui::Text("Action_GameMenu %i", controller_button_held(steam_c, handles[i], DA::Action_GameMenu));

    ImGui::Text("Menu_Up %i", controller_button_held(steam_c, handles[i], DA::Menu_Up));
    ImGui::Text("Menu_Down %i", controller_button_held(steam_c, handles[i], DA::Menu_Down));
    ImGui::Text("Menu_Left %i", controller_button_held(steam_c, handles[i], DA::Menu_Left));
    ImGui::Text("Menu_Right %i", controller_button_held(steam_c, handles[i], DA::Menu_Right));
    ImGui::Text("Menu_Select %i", controller_button_held(steam_c, handles[i], DA::Menu_Select));
    ImGui::Text("Menu_Cancel %i", controller_button_held(steam_c, handles[i], DA::Menu_Cancel));

    // ImGui::Text("Action_GameCancel %i", controller_button_held(steam_c, handles[i], Action_GameCancel));
    // ImGui::Text("Action_GameMenu %i", controller_button_held(steam_c, handles[i], Action_GameMenu));
    auto analog = controller_axis(r, handles[i], AA::AnalogControls);
    ImGui::Text("Analog: %f %f", analog.x, analog.y);
  }

  ImGui::End();
}

} // namespace game2d