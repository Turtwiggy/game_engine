#pragma once

#include "app/application.hpp"
#include "colour/colour.hpp"
#include "components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

void
init_render_system(const engine::SINGLETON_Application& app, SINGLETON_RendererInfo& ri, std::vector<Texture>& tex);

void
update_render_system(SINGLETON_RendererInfo& ri,
                     const engine::LinearColour& lin_background,
                     std::vector<Texture>& tex,
                     entt::registry& registry);

void
end_frame_render_system(entt::registry& registry);

} // namespace game2d
