#include "pch.hpp"

#include "steam_debug_ui_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"

namespace game2d {
using DA = DigitalAction;
using AA = AnalogAction;
using AS = ActionSet;

void
update_steam_debug_ui_system(entt::registry& r)
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
  if (digital_action_handles[(int)DA::Game_Up] == 0) {
    init_steam_input_actions(r);
    ImGui::Text("DigitalActionHandles are not loaded...");
    ImGui::End();
    return;
  }

  // static auto mode = ActionSet::ActionSet_GameControls;
  // static auto modes = engine::enum_class_to_vec_str<ActionSet>();
  // WomboComboIn combo_in(modes);
  // combo_in.label = "ActionSet";
  // combo_in.current_index = static_cast<int>(mode);
  // WomboComboOut combo_out = draw_wombo_combo(combo_in);
  // if (combo_in.current_index != combo_out.selected)
  //   mode = static_cast<AS>(combo_out.selected);

  // bool escape_pressed = get_key_down(input_c, SDL_SCANCODE_ESCAPE);
  // m_pGameEngine->BIsControllerActionActive( eControllerDigitalAction_PauseMenu ) ||
  // m_pGameEngine->BIsControllerActionActive( eControllerDigitalAction_MenuCancel ) )

  // auto& action_set_menu = steam_c.action_set_handles[(int)AS::eControllerActionSet_MenuControls];
  // auto& action_set = steam_c.action_set_handles[(int)combo_out.selected];
  // auto& handles = steam_c.handles;
  // auto action_set = AS::ActionSet_GameControls;

  for (int i = 0; i < steam_c.n_active; i++) {
    const auto handle = steam_c.handles[i];
    const auto label = std::format("Controller: {}", i);
    ImGui::SeparatorText(label.c_str());
    // SteamInput()->ActivateActionSet(handle, action_set);

    for (int j = 0; j < static_cast<int>(DigitalAction::count); j++) {
      const auto act = static_cast<DigitalAction>(j);
      const auto act_str = std::string(magic_enum::enum_name<DA>(act));
      bool held = controller_button_held(steam_c, handle, act);
      ImGui::Text("%s %i", act_str.c_str(), (int)held);
    }

    // ImGui::Text("Action_GameCancel %i", controller_button_held(steam_c, handles[i], Action_GameCancel));
    // ImGui::Text("Action_GameMenu %i", controller_button_held(steam_c, handles[i], Action_GameMenu));
    auto l_analog = controller_axis(r, handle, AA::LAnalogControls);
    auto r_analog = controller_axis(r, handle, AA::RAnalogControls);
    ImGui::Text("LAnalog: %f %f", l_analog.x, l_analog.y);
    ImGui::Text("RAnalog: %f %f", r_analog.x, r_analog.y);
  }

  ImGui::End();
}

} // namespace game2d