#pragma once

#include "app/application.hpp"
#include "events/components.hpp"

namespace game2d {

void
init_input_system(entt::registry& r);

void
update_input_system(engine::SINGLETON_Application& app, SINGLETON_InputComponent& input);

} // namespace game2d