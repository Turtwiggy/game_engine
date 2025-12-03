#include "pch.hpp"

#include "ui_main_menu_controllerinfo_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/std/string/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"

namespace game2d {

bool
handle_is_connected(const SINGLE_SteamConnectedControllers& steam_c, const InputHandle_t handle)
{
  if (handle == 0)
    return false;
  auto it = std::find(steam_c.handles.begin(), steam_c.handles.end(), handle);
  return it != steam_c.handles.end();
};

bool
handle_is_joined(const SINGLE_SteamControllerGameState& ui_c, const InputHandle_t handle)
{
  if (handle == 0)
    return false;
  auto it = std::find(ui_c.handles_that_want_to_play.begin(), ui_c.handles_that_want_to_play.end(), handle);
  return it != ui_c.handles_that_want_to_play.end();
};

void
assign_handle_to_ui(SINGLE_SteamControllerGameState& ui_c, InputHandle_t handle)
{
  if (handle_is_joined(ui_c, handle))
    return;

  for (int i = 0; i < ui_c.players; i++) {
    if (ui_c.handles_that_want_to_play[i] != 0)
      continue;
    ui_c.handles_that_want_to_play[i] = handle;
    // ui_c.handles_that_want_to_play_joined_this_frame.push_back(handle);
    break;
  }
};

std::vector<InputHandle_t>
handles_ordered_by_joined_then_connected(entt::registry& r)
{
  auto& ui_c = get_first_component<SINGLE_SteamControllerGameState>(r);
  auto& steam_connected_c = get_first_component<SINGLE_SteamConnectedControllers>(r);

  auto join_handles = non_zero_handles(ui_c.handles_that_want_to_play);
  auto conn_handles = non_zero_handles(connected_but_not_joined_controllers(steam_connected_c, ui_c));

  std::vector<InputHandle_t> handles;
  handles.insert(handles.end(), join_handles.begin(), join_handles.end());
  handles.insert(handles.end(), conn_handles.begin(), conn_handles.end());

  for (int i = handles.size(); i < ui_c.players; i++)
    handles.push_back(0);

  return handles;
};

std::vector<InputHandle_t>
connected_but_not_joined_controllers(const SINGLE_SteamConnectedControllers& steam_c,
                                     const SINGLE_SteamControllerGameState& ui_c)
{
  std::vector<InputHandle_t> connected_but_not_joined;
  for (int i = 0; i < steam_c.n_active; i++) {
    const InputHandle_t handle = steam_c.handles[i];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(ui_c, handle);
    if (connected && !joined)
      connected_but_not_joined.push_back(steam_c.handles[i]);
  }
  return connected_but_not_joined;
};

void
add_text_centered(entt::registry& r, ImDrawList* draw_list, const std::string text, const ImVec2 pos, const int alpha)
{
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_size = (float)FontSizes::SIZE_12 * font_scale;
  auto* font = get_inter_font(r);

  ImGui::PushFont(font, font_size);
  const auto text_wh = ImGui::CalcTextSize(text.c_str());
  const auto text_pos = pos - ImVec2{ 0.5f * text_wh.x, 0.5f * text_wh.y };
  draw_list->AddText(text_pos, IM_COL32(255, 255, 255, alpha), text.c_str());
  ImGui::PopFont();
};

std::string
get_str_for_da(const SINGLE_SteamMappings& steam_c, const InputHandle_t handle, const DigitalAction da)
{
  // DigitalAction
  const auto& digital_action_handles = steam_c.digital_action_handles;
  const auto h = digital_action_handles[(int)da];

  // ActionSet
  const auto& actionset_handles = steam_c.action_set_handles;
  const auto as = actionset_handles[(int)AS::ActionSet_GameControls];

  // This would return the names of the actions on steam
  // return SteamInput()->GetStringForDigitalActionName(h);

  EInputActionOrigin origins[STEAM_INPUT_MAX_ORIGINS];
  auto n_origins = SteamAPI_ISteamInput_GetDigitalActionOrigins(SteamAPI_SteamInput(), handle, as, h, origins);

  if (n_origins > 0) {
    // use the first origin keyname
    EInputActionOrigin origin = origins[0];
    const char* keyname = SteamInput()->GetStringForActionOrigin(origin);

    // return things like "B Button";
    const auto button_str = std::string(keyname);

    // if it has button, remove that
    const auto pos = to_lower(button_str).find(" button");
    if (pos != std::string::npos)
      return button_str.substr(0, pos);

    return button_str;
  }

  // TODO: work out when some controllers are returning this
  return "...";
};

} // namespace game2d