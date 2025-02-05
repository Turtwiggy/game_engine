#include "ui_main_menu_playerjoin_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"

namespace game2d {

bool
handle_is_connected(const SINGLE_SteamControllers& steam_c, const InputHandle_t handle)
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
  auto it = std::find(ui_c.handles.begin(), ui_c.handles.end(), handle);
  return it != ui_c.handles.end();
};

bool
handle_joined_this_frame(const SINGLE_SteamControllerGameState& ui_c, const InputHandle_t handle)
{
  if (handle == 0)
    return false;
  const auto& hs = ui_c.handles_joined_this_frame;
  auto it = std::find(hs.begin(), hs.end(), handle);
  return it != hs.end();
};

void
assign_handle_to_ui(SINGLE_SteamControllerGameState& ui_c, InputHandle_t handle)
{
  if (handle_is_joined(ui_c, handle))
    return;

  for (int i = 0; i < ui_c.players; i++) {
    if (ui_c.handles[i] != 0)
      continue;
    ui_c.handles[i] = handle;
    ui_c.handles_joined_this_frame.push_back(handle);
    break;
  }
};

void
unassign_handle_from_ui(SINGLE_SteamControllerGameState& ui_c, InputHandle_t handle)
{
  if (!handle_is_joined(ui_c, handle))
    return;
  auto it = std::find(ui_c.handles.begin(), ui_c.handles.end(), handle);
  const auto idx = static_cast<int>(it - ui_c.handles.begin());
  ui_c.handles[idx] = 0;
};

std::vector<InputHandle_t>
connected_but_not_joined_controllers(const SINGLE_SteamControllers& steam_c, const SINGLE_SteamControllerGameState& ui_c)
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

} // namespace game2d