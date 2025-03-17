#pragma once

#include "modules/steam_input/steam_input_components.hpp"

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

void
init_steam_input(entt::registry& r);

void
init_steam_input_actions(entt::registry& r);

void
update_steam_input(entt::registry& r);

//

bool
controller_button_down(const SINGLE_SteamControllers& steam_c, InputHandle_t handle, const DA dAction);

bool
controller_button_held(const SINGLE_SteamControllers& steam_c, InputHandle_t handle, const DA dAction);

bool
controller_button_release(const SINGLE_SteamControllers& steam_c, InputHandle_t handle, const DA dAction);

glm::vec2
controller_axis(entt::registry& r, InputHandle_t handle, AA aAction);

std::vector<InputHandle_t>
non_zero_handles(const std::vector<InputHandle_t>& handles);

//

void
set_all_steam_controller_action_set(SINGLE_SteamControllers& steam_c, AS set);

} // namespace game2d