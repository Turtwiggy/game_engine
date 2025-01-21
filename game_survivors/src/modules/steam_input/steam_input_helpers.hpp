#pragma once

#include "modules/steam_input/steam_input_components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

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

std::vector<InputHandle_t>
unassigned_steam_input_handles(entt::registry& r);

std::optional<InputHandle_t>
aquire_unused_steam_input_handle(entt::registry& r);

} // namespace game2d