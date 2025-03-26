#pragma once

#include "modules/core_renderer/components.hpp"
#include "modules/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/ui_scene_select/scene_select_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
update_input_for_select_ui(entt::registry& r, SINGLE_SelectSceneData& ui_c);

void
draw_select_header(entt::registry& r, SINGLE_RendererInfo& ri_c);

void
update_countdown(SINGLE_SelectSceneData& data_c, bool everyone_confirmed, float dt);

void
update_countdown_ui(entt::registry& r, const SINGLE_SelectSceneData& data_c);

void
update_countdown_to_next_scene(entt::registry& r,
                               const SINGLE_SelectSceneData& data_c,
                               const std::vector<ShipHullData>& sorted_hulls);

} // namespace game2d