#pragma once

#include "app/application.hpp"
#include "colour/colour.hpp"
#include "modules/renderer/components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

void
init_render_system(const engine::SINGLETON_Application& app, entt::registry& r, SINGLETON_RendererInfo& ri);

void
update_render_system(entt::registry& r);

void
end_frame_render_system(entt::registry& registry);

} // namespace game2d
