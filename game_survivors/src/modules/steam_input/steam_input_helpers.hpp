#pragma once

#include "modules/steam_input/steam_input_components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
init_steam_input(entt::registry& r);

void
init_steam_input_actions(entt::registry& r);

void
update_steam_input(entt::registry& r);

//

bool
controller_button_held(SINGLE_SteamControllers steam_c, InputHandle_t handle, DA dAction);

glm::vec2
controller_axis(entt::registry& r, InputHandle_t handle, AA aAction);

//

void
set_steam_controller_action_set(entt::registry& r, InputHandle_t handle, AS set);

} // namespace game2d