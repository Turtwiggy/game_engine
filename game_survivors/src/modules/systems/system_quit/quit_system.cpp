#include "pch.hpp"

#include "quit_system.hpp"

#include "modules/core/ui/ui_common_helpers.hpp"
#include "quit_components.hpp"

namespace game2d {

void
update_quit_system(entt::registry& r, engine::SINGLE_Application& app)
{
  process_requests<RequestQuitApplication>(r, [&app](const auto& req) {
    app.running = false; //
  });
}

} // namespace game2d