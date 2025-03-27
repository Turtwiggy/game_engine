#pragma once

#include "imgui.h"

#include "modules/steam_input/steam_input_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"

namespace game2d {

// Connected (steamsdk knows about handle)
bool
handle_is_connected(const SINGLE_SteamControllers& steam_c, const InputHandle_t handle);

// Joined (ui knows about handle)
bool
handle_is_joined(const SINGLE_SteamControllerGameState& ui_c, const InputHandle_t handle);

bool
handle_joined_this_frame(const SINGLE_SteamControllerGameState& ui_c, const InputHandle_t handle);

void
assign_handle_to_ui(SINGLE_SteamControllerGameState& ui_c, InputHandle_t handle);

void
unassign_handle_from_ui(SINGLE_SteamControllerGameState& ui_c, InputHandle_t handle);

std::vector<InputHandle_t>
connected_but_not_joined_controllers(const SINGLE_SteamControllers& steam_c, const SINGLE_SteamControllerGameState& ui_c);

void
add_text_centered(entt::registry& r, ImDrawList* draw_list, const std::string text, const ImVec2 pos, const int alpha);

std::string
get_str_for_da(const SINGLE_SteamControllers& steam_c, const InputHandle_t handle, const DigitalAction da);

} // namespace game2d